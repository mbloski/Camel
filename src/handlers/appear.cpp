/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

void handler::appear_approach(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    // TODO: check if 1_27 can contain any other data than player IDs (since its last byte is BYTE_COMMA)
    size_t expected_ids = packet->bytes_left() / 2;

    std::vector<uint16_t> ids;
    for (size_t i = 0; i < expected_ids; ++i)
    {
        ids.push_back(packet->get_int(2));
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_APPEAR, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_int(ids.size(), 1);
    /* Do we really need to loop through all the players?
       Might want to optimize this in the future. */
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->position.map == client->character->position.map && c->id != client->id)
        {
            size_t dist = std::abs(client->character->position.x - c->character->position.x) + std::abs(client->character->position.y - c->character->position.y);
            if (dist >= 11 && dist <= 13)
            {
                for (uint16_t id : ids)
                {
                    if (c->id == id)
                    {
                        add_player_to_packet_builder(server, client, c.get());
                    }
                }
            }
        }
    }
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->Send();
}
