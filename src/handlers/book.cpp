/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

void handler::open_book(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);
    out::info("%d", id);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->id == id)
        {
            send_book(client, c.get());
            return;
        }
    }
}
