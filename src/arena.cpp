/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "arena.hpp"

Arena::Arena(eodata::EMF::map_t map, size_t spawn_rate)
{
    this->map = map;
    this->spawn_rate = spawn_rate;

    if (this->map.id == 0)
    {
        return;
    }

    /* Find spawn points */
    for (std::pair<std::pair<uint8_t, uint8_t>, eodata::EMF::map_t::tile> i : this->map.tiles)
    {
        if (i.second.type == eodata::EMF::TILE_NOGHOST)
        {
            uint8_t x = i.first.first;
            uint8_t y = i.first.second;

            /* We're looking for arena tiles surrounded by at least 3 non-walkable tiles.
               We can deduce spawn point coords this way. */
            uint8_t tile_score = this->map.is_walkable(x - 1, y)
                               + this->map.is_walkable(x + 1, y)
                               + this->map.is_walkable(x, y - 1)
                               + this->map.is_walkable(x, y + 1);

            if (tile_score > 1)
            {
                continue;
            }

            this->spawn_points.push_back(std::make_pair(x, y));
        }
    }

    /* Find arena */
    for (std::pair<std::pair<uint8_t, uint8_t>, eodata::EMF::map_t::tile> i : this->map.tiles)
    {
        if (i.second.type == eodata::EMF::TILE_ARENA)
        {
            uint8_t x = i.first.first;
            uint8_t y = i.first.second;

            this->arena_tiles.push_back(std::make_pair(x, y));
        }
    }

    out::info("Found %d arena spawn points and %d arena tiles on map %d.", this->spawn_points.size(), this->arena_tiles.size(), this->map.id);
    if (this->spawn_points.size() > this->arena_tiles.size())
    {
        out::warning("Arena on map %d is too small!", this->map.id);
    }
}

Arena::~Arena()
{

}

void Arena::TryLaunch(Server *server)
{
#ifdef DEBUG
    server->MapAnnounce(this->map.id, "[debug] Server called Arena::TryLaunch for this map.");
#endif // DEBUG

    bool failed = false;

    if (this->participants.size() >= 3)
    {
        this->delayed = true;
    }

    if (this->arena_tiles.empty() || this->spawn_points.size() > this->arena_tiles.size())
    {
        failed = true;
    }

    // TODO: hang arenas with admin commands

    if (!failed)
    {
        this->Launch(server);
    }
    else
    {
#ifdef DEBUG
        server->MapAnnounce(this->map.id, "[debug] Unfortunately, conditions to launch arena on this map were not met.");
#endif // DEBUG
    }
}

void Arena::Launch(Server *server)
{
    uint8_t howmany = 0;
    std::vector<std::pair<uint8_t, uint8_t>> spawns;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->position.map == this->map.id)
        {
            std::pair<uint8_t, uint8_t> coords = std::make_pair(c->character->position.x, c->character->position.y);
            if (std::find(this->spawn_points.begin(), this->spawn_points.end(), coords) != this->spawn_points.end())
            {
                ++howmany;
#ifdef DEBUG
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_SERVER);
                c->get_packet_builder()->add_string("[debug] I have processed your request to join arena.");
                c->Send();
#endif // DEBUG

                if (!this->delayed)
                {
                    std::pair<uint8_t, uint8_t> randpos;
                    do
                    {
                        randpos = this->arena_tiles.at(server->random.get_int<uint8_t>(0, this->arena_tiles.size() - 1));
                    }
                    while (std::find(spawns.begin(), spawns.end(), randpos) != spawns.end());
                    spawns.push_back(randpos);
                    
                    server->Warp(c.get(), this->map.id, randpos.first, randpos.second, 0, true);
                    if (std::find(this->participants.begin(), this->participants.end(), c) == this->participants.end())
                    {
                        this->participants.push_back(c);
                    }
                    c->onarena = true;
                    c->session_vals["arena_kills"] = 0;
                }
            }
        }
    }

    if (this->delayed && howmany > 0)
    {
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->position.map == this->map.id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ARENA, eoencoding::PACTION_DROP);
                c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                c->Send();
            }
        }
    }

    if (this->delayed)
    {
        /* Better luck next time. */
        this->delayed = false;
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->position.map == this->map.id)
        {
            if (howmany > 0)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ARENA, eoencoding::PACTION_USE);
                c->get_packet_builder()->add_int(howmany, 1);
                c->Send();
            }
        }
    }
}

void Arena::Cleanup(Server *server)
{
#ifdef DEBUG
    size_t players_before_cleanup = this->participants.size();
    server->MapAnnounce(this->map.id, "[debug] " + std::to_string(players_before_cleanup) + " player(s) on arena. Performing cleanup.");
#endif // DEBUG
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (!c->onarena)
        {
            std::vector<std::shared_ptr<EOClient>>::iterator pos = std::find(this->participants.begin(), this->participants.end(), c);
            if (pos != this->participants.end())
            {
#ifdef DEBUG
                server->MapAnnounce(this->map.id, "[debug] " + c->character->name + " has left arena");
#endif // DEBUG
                this->participants.erase(pos);
            }
        }
    }
#ifdef DEBUG
    server->MapAnnounce(this->map.id, "[debug] " + std::to_string(this->participants.size()) + " player(s) left after cleanup.");
#endif // DEBUG

    if (this->participants.size() == 1)
    {
       server->MapAnnounce(this->map.id, "The event was aborted, last opponent left -server");
    }
}
