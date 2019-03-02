/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::door(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t x = packet->get_int(1);
    uint8_t y = packet->get_int(1);

    bool authenticated = false;

    if (server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].warp.exists())
    {
        eodata::EMF::map_t map = server->maps->get(client->character->position.map);
        eodata::EMF::map_t::warp &warp = map.tiles[std::make_pair(x, y)].warp;

        if (!client->character->in_range_of(client->character->position.map, x, y) || warp.open)
        {
            return;
        }

        if (warp.type == eodata::EMF::WARP_DOOR)
        {
            authenticated = true;
        }

        // TODO: key protected doors

        if (authenticated)
        {
            warp.open = true;
            // FIXME: optimalization?
            server->maps->update(client->character->position.map, map);
            std::string evargv = std::to_string(client->character->position.map) + " " + std::to_string(x) + " " + std::to_string(y);
            server->evman->add(SchedEvents::EventType::EVENT_ONEFOLD, 3000, event::close_door, nullptr, evargv);

            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->in_range(client->character))
                {
                    c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_DOOR, eoencoding::PACTION_OPEN);
                    c->get_packet_builder()->add_int(x, 1);
                    c->get_packet_builder()->add_int(y, 1);
                    c->get_packet_builder()->add_char(eoencoding::BYTE_NULL);
                    c->Send();
                }
            }
        }
    }
}
