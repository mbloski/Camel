/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::latency(Server *server, EOClient *client)
{
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_MESSAGE, eoencoding::PACTION_PONG);
    client->get_packet_builder()->add_int(2, 2);
    client->Send();
}
