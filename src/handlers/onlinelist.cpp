/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::online_list(Server *server, EOClient *client)
{
    size_t player_count = 0;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && !c->character->hidden)
            ++player_count;
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);
    client->get_packet_builder()->add_int(8, 1);
    client->get_packet_builder()->add_int(player_count, 2);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && !c->character->hidden)
        {
            uint8_t player_icon = c->character->admin; // TODO: party icons
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->title);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_char(c->random.get_int(20, 40)); // ?
            client->get_packet_builder()->add_char(player_icon);
            client->get_packet_builder()->add_int(c->character->class_id, 1);
            client->get_packet_builder()->add_string(util::str_pad(c->character->guild, 3, ' ', util::STR_PAD_RIGHT));
        }
    }
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA); // I hate Apollo.
    client->Send();
}

void handler::online_list_friends(Server *server, EOClient *client)
{
    size_t player_count = 0;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && !c->character->hidden)
            ++player_count;
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);
    client->get_packet_builder()->add_int(10, 1);
    client->get_packet_builder()->add_int(player_count, 2);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && !c->character->hidden)
        {
            client->get_packet_builder()->add_string(c->character->name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        }
    }
    client->Send();
}

void handler::find_player(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string nickname = util::strtolower(packet->get_string());

    if (nickname.length() < 4)
    {
        return;
    }
    else if (nickname.length() > 16)
    {
        nickname = nickname.substr(0, 16);
    }

    bool found = false;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->name == nickname && !c->character->hidden)
        {
            found = true;
            if (c->character->position.map == client->character->position.map)
            {
                client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_PONG);
                break;
            }
            else
            {
                client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_NET3);
                break;
            }
        }
    }

    if (!found)
    {
        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_PING);
    }
    client->get_packet_builder()->add_string(nickname);
    client->Send();
}
