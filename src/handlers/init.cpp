/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

inline size_t calculate_challenge(size_t challenge)
{
    ++challenge;
    return 110905 + (challenge % 9 + 1) * ((11092004 - challenge) % ((challenge % 11 + 1) * 119)) * 119 + challenge % 2004;
}

inline size_t generate_player_id(Server *server, size_t max_connections)
{
    size_t fon = max_connections / 2;
    if (fon % 2 == 0)
    {
        ++fon;
    }
    fon = fon + 2;
    size_t limit = ((fon * 2) * 10) - 1;
    size_t mwk = (limit - fon) / 2;
    size_t wk = 2 * server->random.get_int<size_t>(1, mwk);
    size_t wyz = fon + wk;
    size_t id = wyz * 2;

    return id;
}

void handler::init(Server *server, EOClient *client)
{
    if (client->initialized)
    {
        client->Disconnect();
        return;
    }

    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    size_t player_id;
    bool id_free;

    if (!server->id_cache.empty())
    {
        player_id = server->id_cache.back();
        server->id_cache.pop_back();
    }
    else
    {
        out::info("Client peak reached. Adding new player ID to the pool.");
        do
        {
            player_id = generate_player_id(server, (server->id_cache.size() + server->eoclients.size()) / 2);
            id_free = true;
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->id == player_id)
                {
                    id_free = false;
                    break;
                }
            }
        } while (!id_free);
    }

    client->id = player_id;

    std::vector<uint8_t> version =  {0, 0, 0};
    size_t challenge = packet->get_int(3);
    version[0] = packet->get_int(1);
    version[1] = packet->get_int(1);
    version[2] = packet->get_int(1);
    packet->get_int(2); // 2 unknown bytes
    client->hdid = std::stoull(packet->get_string());

    char version_str[12];
    sprintf(version_str, "%u.%u.%u", version[0], version[1], version[2]);

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);

    if (server->get_database()->IsBanned(client->get_client()->host) || server->get_database()->IsHDIDBanned(client->hdid))
    {
        size_t banlength = 0;
        if (server->get_database()->IsBanned(client->get_client()->host))
        {
            banlength = server->get_database()->GetBanLength(client->get_client()->host);
        }
        else if (server->get_database()->IsHDIDBanned(client->hdid))
        {
            banlength = server->get_database()->GetHDIDBanLength(client->hdid);
        }

        client->get_packet_builder()->add_int(2, 1);
        client->get_packet_builder()->add_char(banlength == 0? 2 : 0);
        uint8_t rel_ban_time = std::ceil(banlength / 60.0);
        client->get_packet_builder()->add_char(banlength == 0? 255 : rel_ban_time > 255? 255 : rel_ban_time);
        client->Send();
        client->Disconnect();
        return;
    }

    if (std::string(version_str) != server->eo_version)
    {
        out::info("Client %u is using an outdated version '%s' (required: '%s').", client->id, version_str, server->eo_version.c_str());

        client->get_packet_builder()->add_char(1);
        std::vector<std::string> version_chunks = util::tokenize(server->eo_version, '.');
        for (std::string chunk : util::tokenize(server->eo_version, '.'))
        {
            size_t chunk_int = atoi(chunk.c_str());
            client->get_packet_builder()->add_char(eoencoding::eoint_encode((uint8_t)chunk_int, 1)[0]);
        }
        client->Send();
        client->Disconnect();
        return;
    }

    std::pair<uint8_t, uint8_t> seqbytes = std::make_pair(client->random.get_int(8, 28), client->random.get_int(7, 17));
    /* Possible range: 50-200 */
    uint8_t startseq = seqbytes.first * 7 - 11 + seqbytes.second - 2;
    client->get_packet_reader()->SetSequence(startseq);

    out::info("New connection initialized: {ID:%u, HDID:%u, SeqByte:%u, Multi:[%u, %u]}", client->id, client->hdid, startseq, client->get_multi().first, client->get_multi().second);

    client->get_packet_builder()->add_char(2);
    client->get_packet_builder()->add_char(seqbytes.first);
    client->get_packet_builder()->add_char(seqbytes.second);
    client->get_packet_builder()->add_char(client->get_multi().first);
    client->get_packet_builder()->add_char(client->get_multi().second);
    client->get_packet_builder()->add_int(client->id, 2);
    client->get_packet_builder()->add_int(calculate_challenge(challenge), 3);
    client->Send();

    REGISTER_HANDLER(client, eoencoding::PFAMILY_CONNECTION, eoencoding::PACTION_ACCEPT, handler::init_ack);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_LOGIN, eoencoding::PACTION_REQUEST, handler::login);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_REQUEST, handler::name_availability);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_CREATE, handler::register_account);

    client->initialized = true;
}

void handler::init_ack(Server *server, EOClient *client)
{
    uint8_t emulti_d = client->get_packet_reader()->get_int(2);
    uint8_t emulti_e = client->get_packet_reader()->get_int(2);
    size_t client_id = client->get_packet_reader()->get_int(2);

    if (emulti_e != client->get_multi().first || emulti_d != client->get_multi().second || client_id != client->id)
    {
#ifdef DEBUG
        out::debug("Client %u sent wrong data about itself in the INIT ACK packet.", client->id);
#endif // DEBUG
        client->Disconnect();
    }

    REGISTER_HANDLER(client, eoencoding::PFAMILY_CONNECTION, eoencoding::PACTION_PING, handler::pong);
}
