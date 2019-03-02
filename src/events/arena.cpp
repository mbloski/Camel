/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "events.hpp"

namespace event
{
    void launch_arena(Server *server, EOClient *client, std::string args)
    {
        server->arenas[stoul(args)]->TryLaunch(server);
    }
};
