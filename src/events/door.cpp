/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "events.hpp"

namespace event
{
    void close_door(Server *server, EOClient *client, std::string args)
    {
        std::vector<std::string> _args = util::tokenize(args, ' ');
        uint16_t map = stoi(_args[0]);
        uint8_t x = stoi(_args[1]);
        uint8_t y = stoi(_args[2]);

        /* Safe to assume the warp is there. No checks needed. */
        eodata::EMF::map_t umap = server->maps->get(map);
        eodata::EMF::map_t::warp &warp = umap.tiles[std::make_pair(x, y)].warp;
        if (warp.open)
        {
            warp.open = false;
            server->maps->update(map, umap);
        }
    }
};
