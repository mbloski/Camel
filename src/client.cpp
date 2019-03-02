/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "client.hpp"

EOClient::EOClient(std::shared_ptr<Config> config, Client *client)
{
    this->config = config;
    this->client = client;
    uint8_t multi_e = this->random.get_int<uint8_t>(6, 12);
    uint8_t multi_d = this->random.get_int<uint8_t>(6, 12);
    this->packet_builder = std::shared_ptr<PacketBuilder>(new PacketBuilder(multi_e));
    this->packet_reader = std::shared_ptr<PacketReader>(new PacketReader(multi_d));

    this->handler_manager = std::shared_ptr<HandlerManager>(new HandlerManager(this));
    this->handler_manager->Register(std::make_pair(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT), handler::init);
    if (this->config->get_bool("allow_premature_onlinelist"))
    {
        /* a handy way to grab live player list by third party software */
        this->handler_manager->Register(std::make_pair(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_REQUEST), handler::online_list);
    }

    this->sequence_validator = this->config->get_bool("seqbyte_validator");
    this->ping_delay = this->config->get_number<size_t>("ping_delay");

    if (this->ping_delay == 0)
    {
        out::warning("ping_delay is set to 0. Defaulting to 30");
        this->ping_delay = 30;
    }

    this->alive_since = std::time(0);
    this->last_ping_time = this->alive_since;
}

EOClient::~EOClient()
{
    this->ingame = false;
    if (this->character != nullptr)
    {
        delete this->character;
    }

    if (this->id == 0)
    {
        out::info("Uninitialized session destroyed. Lasted %u seconds.", this->get_age());
    }
    else
    {
        out::info("Session %u destroyed. Lasted %u seconds.", this->id, this->get_age());
    }

    delete this->client;
}

bool EOClient::Send()
{
    return this->client->Send(this->get_packet_builder()->get());
}

std::string EOClient::Recv()
{
    std::vector<uint8_t> tmplen;
    tmplen.push_back((unsigned char)this->client->Recv(1)[0]);
    tmplen.push_back((unsigned char)this->client->Recv(1)[0]);

    if (tmplen[0] == 0 || tmplen[1] == 0)
    {
        return "";
    }

    unsigned int len = eoencoding::eoint_decode(tmplen);

    std::string data = this->client->Recv(len);

    this->packet_reader->Load(data, this->initialized? false : true);
    if (this->get_packet_reader()->get_packet_mode() == eoencoding::PACKET_ENCODED)
    {
        this->get_packet_reader()->increase_sequence_counter();

        /* If we care about the sequence byte and the client sent a wrong one, ignore the packet. */
        if (this->sequence_validator && this->get_packet_reader()->get_sequence() != this->get_packet_reader()->get_received_sequence())
        {
#ifdef DEBUG
            out::debug("Received a wrong sequence byte %u from client %u (expected %u)", this->get_packet_reader()->get_received_sequence(), this->id, this->get_packet_reader()->get_sequence());
#endif // DEBUG
            return "";
        }
    }

    return data;
}

std::string EOClient::Tick()
{
    /* If there's still any data left in queues, try to solve this issue first. */
    if (!this->client->retryqueue.empty())
    {
        this->client->Send(this->client->retryqueue.front());
        this->client->retryqueue.pop();
    }

    std::string ret = this->Recv();

    time_t curr_time = std::time(0);
    if (ret.empty())
    {
        if (this->initialized && size_t(curr_time - this->last_ping_time) >= this->ping_delay)
        {
            this->last_ping_time = curr_time;

            uint16_t s1 = this->random.get_int<uint16_t>(200, 300);
            uint8_t s2 = this->random.get_int<uint8_t>(100, 150);
            /* Possible range: 50 - 200 */
            uint8_t newsequence = s1 - s2;

            this->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CONNECTION, eoencoding::PACTION_PLAYER);
            this->get_packet_builder()->add_int(s1, 2);
            this->get_packet_builder()->add_int(s2, 1);
            this->Send();

            this->get_packet_reader()->SetSequence(newsequence);

            /* We decrement this in the ping handler so it should always end up being 0 if everything works well.
               If too many unreplied pings, the server will disconnect the client. */
            ++this->missed_pings;
        }
    }

    return ret;
}

void EOClient::Disconnect()
{
    this->client->connected = false;
}
