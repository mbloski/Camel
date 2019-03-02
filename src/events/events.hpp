/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __EVENTS_HPP_INCLUDED
#define __EVENTS_HPP_INCLUDED

#include "../server.hpp"
#include "../client.hpp"

namespace event
{
    void close_door(Server *server, EOClient *client, std::string args);
    void launch_arena(Server *server, EOClient *client, std::string args);
    void map_drop_effects(Server *server, EOClient *client, std::string args);
    void autoupdate(Server *server, EOClient *client, std::string args);
};

#endif // __EVENTS_HPP_INCLUDED
