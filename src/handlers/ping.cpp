/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::pong(Server *server, EOClient *client)
{
    /* The client replied. Let's forgive any previous missed pings (if any)
       because we only want to make sure the client is still alive and it just proved it. */
    client->missed_pings = 0;
}
