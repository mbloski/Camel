/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::attack(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t direction = packet->get_int(1);
    size_t timestamp = packet->get_int(3);

    std::pair<uint8_t, uint8_t> attack_pos;
    attack_pos.first = client->character->position.x;
    attack_pos.second = client->character->position.y;
    switch (direction)
    {
    case 0:
        ++attack_pos.second;
        break;
    case 1:
        --attack_pos.first;
        break;
    case 2:
        --attack_pos.second;
        break;
    case 3:
        ++attack_pos.first;
        break;
    }

    if (client->character->sitting > 0 || direction > 3)
    {
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character, 13) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ATTACK, eoencoding::PACTION_PLAYER);
            c->get_packet_builder()->add_int(client->id, 2);
            c->get_packet_builder()->add_int(direction, 1);
            c->Send();
        }
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (client->onarena && c->ingame && c->onarena && c->character->position.map == client->character->position.map && server->maps->get(c->character->position.map).tiles[std::make_pair(c->character->position.x, c->character->position.y)].type == eodata::EMF::TILE_ARENA)
        {
            if (c->character->position.x == attack_pos.first && c->character->position.y == attack_pos.second)
            {
#ifdef DEBUG
                server->MapAnnounce(client->character->position.map, "[debug] " +client->character->name + " attacks " + c->character->name);
#endif // DEBUG

                bool kill = false;
                bool win = false;
                uint16_t map = 0;
                std::shared_ptr<EOClient> victim = c;

                for (std::shared_ptr<Arena> arena : server->arenas)
                {
                    std::vector<std::shared_ptr<EOClient>>::iterator pos = std::find(arena->participants.begin(), arena->participants.end(), victim);
                    if (pos != arena->participants.end())
                    {
                        victim->onarena = false;
                        kill = true;
                        client->character->arena_kills += 1;
                        ++client->session_vals["arena_kills"];
                        server->Warp(victim.get(), arena->get_map_id(), server->maps->get(arena->get_map_id()).relogx, server->maps->get(arena->get_map_id()).relogy, 0, true);
                        if (arena->participants.size() == 2)
                        {
                            client->onarena = false;
                            win = true;
                            client->character->arena_wins += 1;
                            server->Warp(client, arena->get_map_id(), server->maps->get(arena->get_map_id()).relogx, server->maps->get(arena->get_map_id()).relogy, 0, true);
                        }

                        arena->Cleanup(server);
                        map = arena->get_map_id();
                        break;
                    }
                }

                victim->character->arena_deaths += 1;
                if (server->arena_class_hierarchy.size() > 0)
                {
                    bool applies = false;
                    for (std::pair<size_t, size_t> i : server->arena_class_hierarchy)
                    {
                        if (i.first == client->character->class_id)
                        {
                            applies = true;
                            break;
                        }
                    }

                    if (applies)
                    {
                        client->character->class_id = server->arena_class_hierarchy.begin()->first;
                        for (std::pair<size_t, size_t> i : server->arena_class_hierarchy)
                        {
                            if (client->character->arena_kills >= i.second)
                            {
                                client->character->class_id = i.first;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }

                if (win)
                {
                    for (std::shared_ptr<EOClient> c : server->eoclients)
                    {
                        if (c->ingame && c->character->position.map == map)
                        {
                            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ARENA, eoencoding::PACTION_ACCEPT);
                            c->get_packet_builder()->add_string(client->character->name);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_int(client->session_vals["arena_kills"], 4);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_string(client->character->name);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_string(victim->character->name);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->Send();
                        }
                    }

                    if (server->arena_win_gold > 0)
                    {
                        server->AddItem(client, 1, server->arena_win_gold);
                    }

                    if (server->arena_win_exp > 0)
                    {
                        server->AddExp(client, server->arena_win_exp);
                    }
                }
                else if (kill)
                {
                    for (std::shared_ptr<EOClient> c : server->eoclients)
                    {
                        if (c->ingame && c->character->position.map == map)
                        {
                            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ARENA, eoencoding::PACTION_SPEC);
                            c->get_packet_builder()->add_int(0, 2);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_int(0, 1);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_int(client->session_vals["arena_kills"], 4);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_string(client->character->name);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->get_packet_builder()->add_string(victim->character->name);
                            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                            c->Send();
                        }
                    }

                    if (server->arena_kill_gold > 0)
                    {
                        server->AddItem(client, 1, server->arena_kill_gold);
                    }

                    if (server->arena_kill_exp > 0)
                    {
                        server->AddExp(client, server->arena_kill_exp);
                    }
                }

                return;
            }
        }
    }
}
