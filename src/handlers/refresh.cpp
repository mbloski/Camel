/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

void handler::refresh(Server *server, EOClient *client)
{
    uint8_t ids = 0;
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_REFRESH, eoencoding::PACTION_REPLY);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        /* Is this loop really necessary? TODO: maybe some methods to reserve PacketBuilder bytes to fill them later on. */
        if (c->ingame && client->character->in_range(c->character))
        {
            ++ids;
        }
    }
    client->get_packet_builder()->add_int(ids, 1);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && client->character->in_range(c->character))
        {
            add_player_to_packet_builder(server, client, c.get());
        }
    }
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    // npcs here
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    add_item_drops_in_range_to_packet_builder(server, client);
    client->Send();
}
