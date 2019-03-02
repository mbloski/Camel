/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::emote(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t emote = packet->get_int(1);

    if (emote == 0 || emote == 11 || emote == 12 || emote == 13 || emote > 15)
    {
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_EMOTE, eoencoding::PACTION_PLAYER);
            c->get_packet_builder()->add_int(client->id, 2);
            c->get_packet_builder()->add_int(emote, 1);
            c->Send();
        }
    }
}
