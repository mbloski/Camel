/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

std::vector<uint16_t> new_in_range(Server *server, EOClient *client)
{
    std::vector<uint16_t> ret;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && client->character->in_range(c->character, 15) && c->id != client->id)
        {
            uint8_t dir = client->character->position.direction;
            uint8_t xdir;
            uint8_t ydir;
            if (client->character->position.x - c->character->position.x > 0)
            {
                xdir = 3;
            }
            else
            {
                xdir = 1;
            }

            if (client->character->position.y - c->character->position.y > 0)
            {
                ydir = 0;
            }
            else
            {
                ydir = 2;
            }

            if (dir != xdir && dir != ydir)
            {
                size_t dist = std::abs(client->character->position.x - c->character->position.x) + std::abs(client->character->position.y - c->character->position.y);
                if (client->character->position.map == c->character->position.map && dist >= 11 && dist <= 13)
                {
                    ret.push_back(c->id);
                }
            }
        }
    }

    return ret;
}

std::vector<std::pair<size_t, eodata::EMF::drop_t>> new_items_in_range(Server *server, EOClient *client)
{
    std::vector<std::pair<size_t, eodata::EMF::drop_t>> ret;
    for (std::pair<size_t, eodata::EMF::drop_t> i : server->maps->get(client->character->position.map).drops)
    {
        size_t drop_id = i.first;
        eodata::EMF::drop_t drop = i.second;

        if (drop.archived)
        {
            continue;
        }

        if (client->character->in_range_of(client->character->position.map, drop.pos.first, drop.pos.second))
        {
            uint8_t dir = client->character->position.direction;
            uint8_t xdir;
            uint8_t ydir;
            if (client->character->position.x - drop.pos.first > 0)
            {
                xdir = 3;
            }
            else
            {
                xdir = 1;
            }

            if (client->character->position.y - drop.pos.second > 0)
            {
                ydir = 0;
            }
            else
            {
                ydir = 2;
            }

            if (dir != xdir && dir != ydir)
            {
                size_t dist = std::abs(client->character->position.x - drop.pos.first) + std::abs(client->character->position.y - drop.pos.second);
                if (dist >= 11 && dist <= 13)
                {
                    ret.push_back(std::make_pair(drop_id, drop));
                }
            }
        }
    }

    return ret;
}

void handler::walk(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t direction = packet->get_int(1);
    size_t timestamp = packet->get_int(3);
    uint8_t x = packet->get_int(1);
    uint8_t y = packet->get_int(1);
    bool warp = false;

    if (client->character->admin < 2 && packet->get_action() == eoencoding::PACTION_ADMIN)
    {
        /* Nice try hax0r */
        client->Disconnect();
        return;
    }

    uint8_t curr_x = client->character->position.x;
    uint8_t curr_y = client->character->position.y;

    if (curr_x == x && curr_y == y)
    {
        return;
    }

    if (packet->get_action() != eoencoding::PACTION_ADMIN && (client->character->sitting > 0 || direction > 3 || (std::abs(curr_x - x) + std::abs(curr_y - y)) > 1 || !server->maps->get(client->character->position.map).is_walkable(x, y)))
    {
        if ((std::abs(curr_x - x) + std::abs(curr_y - y)) > 5)
        {
            handler::refresh(server, client);
        }
        return;
    }

    if (packet->get_action() == eoencoding::PACTION_PLAYER)
    {
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character) && c->id != client->id && c->character->position.x == x && c->character->position.y == y)
            {
                return;
            }
        }
    }

    if (packet->get_action() == eoencoding::PACTION_SPEC)
    {
        if (std::time(0) - client->session_vals["specwalk_time"] < 5 || server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].type == eodata::EMF::TILE_NOGHOST || server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].type == eodata::EMF::TILE_ARENA)
        {
            if ((std::abs(curr_x - x) + std::abs(curr_y - y)) > 2)
            {
                handler::refresh(server, client);
            }
            return;
        }

        client->session_vals["specwalk_time"] = std::time(0);
    }

    if (server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].warp.exists())
    {
        warp = true;
        eodata::EMF::map_t::warp warp = server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].warp;

        if (warp.level > client->character->level || !warp.open)
        {
            return;
        }

        server->Warp(client, warp.map, warp.x, warp.y, 0);
    }
    else
    {
        client->character->position.direction = direction;
        client->character->position.x = x;
        client->character->position.y = y;

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WALK, eoencoding::PACTION_REPLY);
        for (uint16_t id : new_in_range(server, client))
        {
            client->get_packet_builder()->add_int(id, 2);
        }
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        // new npc ids here
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        for (std::pair<size_t, eodata::EMF::drop_t> i : new_items_in_range(server, client))
        {
            client->get_packet_builder()->add_int(i.first, 2);
            client->get_packet_builder()->add_int(i.second.id, 2);
            client->get_packet_builder()->add_int(i.second.pos.first, 1);
            client->get_packet_builder()->add_int(i.second.pos.second, 1);
            client->get_packet_builder()->add_int(i.second.amount, 3);
        }
        client->Send();

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character, 15) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WALK, eoencoding::PACTION_PLAYER);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(direction, 1);
                c->get_packet_builder()->add_int(x, 1);
                c->get_packet_builder()->add_int(y, 1);
                c->Send();
            }
        }
    }
}
