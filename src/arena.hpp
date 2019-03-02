/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __ARENA_HPP_INCLUDED
#define __ARENA_HPP_INCLUDED

#include "server.hpp"
#include "client.hpp"
#include "character.hpp"
#include "eodata/emf.hpp"

class Server;
class EOClient;

class Arena
{
    public:
    Arena(eodata::EMF::map_t map, size_t spawn_rate);
    ~Arena();
    std::vector<std::shared_ptr<EOClient>> participants;

    void TryLaunch(Server *server);
    void Launch(Server *server);
    void Cleanup(Server *server);

    size_t get_map_id() const { return this->map.id; }
    size_t get_spawn_rate() const { return spawn_rate; }

    private:
    eodata::EMF::map_t map;
    size_t spawn_rate = 60;
    bool delayed = false;
    std::vector<std::pair<uint8_t, uint8_t>> spawn_points;
    std::vector<std::pair<uint8_t, uint8_t>> arena_tiles;
};

#endif // __ARENA_HPP_INCLUDED
