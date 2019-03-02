/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "events.hpp"

namespace event
{
    void idle_gold(Server *server, EOClient *client, std::string args)
    {
        server->AddItem(client, 1, server->idle_gold);
    }
};
