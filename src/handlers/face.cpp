/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::face(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t direction = packet->get_int(1);

    if (direction > 3)
    {
        return;
    }

    client->character->position.direction = direction;

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_FACE, eoencoding::PACTION_PLAYER);
            c->get_packet_builder()->add_int(client->id, 2);
            c->get_packet_builder()->add_int(direction, 1);
            c->Send();
        }
    }
}
