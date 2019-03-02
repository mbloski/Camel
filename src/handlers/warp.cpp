/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

void handler::warp_nomap(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t map = packet->get_int(2);
    uint16_t secid = packet->get_int(2);

    if (map == 0 || secid == 0 || map != client->session_vals["warp_tomap"] || secid != client->session_vals["warp_secid"])
    {
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && c-> id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_AVATAR, eoencoding::PACTION_REMOVE);
            c->get_packet_builder()->add_int(client->id);
            if (client->session_vals["warp_anim"] != 0)
            {
                c->get_packet_builder()->add_int(client->session_vals["warp_anim"], 1);
            }
            c->Send();
        }
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);
    client->get_packet_builder()->add_int(3, 1);
    client->get_packet_builder()->add_string(eodata::get_file(server->maps->get(client->session_vals["warp_tomap"]).path));
    client->Send();

    client->session_vals["nirvana"] = 1;
}

void handler::warp(Server *server, EOClient *client)
{
    if (client->session_vals["warp_secid"] == 0)
    {
        return;
    }

    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t map = client->session_vals["warp_forced"] > 0? client->session_vals["warp_tomap"] : packet->get_int(2);
    uint16_t secid;
    uint8_t x = 0;
    uint8_t y = 0;

    bool needs_avatar_remove = false;
    if (!client->session_vals["nirvana"])
    {
        needs_avatar_remove = true;
    }

    bool authorized = false;
    if (map == client->session_vals["warp_tomap"])
    {
        if (client->character->position.map == client->session_vals["warp_tomap"])
        {
            x = client->session_vals["warp_forced"] > 0? client->session_vals["warp_tox"] : packet->get_int(1);
            y = client->session_vals["warp_forced"] > 0? client->session_vals["warp_toy"] : packet->get_int(1);

            if (x == client->session_vals["warp_tox"] && y == client->session_vals["warp_toy"])
            {
                if (client->onarena && server->maps->get(map).tiles[std::make_pair(x, y)].type != eodata::EMF::TILE_ARENA)
                {
                    /* reset arena state if out of arena */
                    client->onarena = false;
                    for (std::shared_ptr<Arena> arena : server->arenas)
                    {
                        arena->Cleanup(server);

                    }
                }

                authorized = true;
            }
        }
        else
        {
            secid = client->session_vals["warp_forced"] > 0? client->session_vals["warp_secid"] : packet->get_int(2);

            if (secid == client->session_vals["warp_secid"])
            {
                authorized = true;
            }
        }
    }

    if (!authorized)
    {
        return;
    }

    if (needs_avatar_remove && client->session_vals["warp_forced_p2"] == 0)
    {
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character) && c-> id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_AVATAR, eoencoding::PACTION_REMOVE);
                c->get_packet_builder()->add_int(client->id, 2);
                if (client->session_vals["warp_anim"] != 0)
                {
                    c->get_packet_builder()->add_int(client->session_vals["warp_anim"], 1);
                }
                c->Send();
            }
        }
    }

    client->character->position.map = map;
    client->character->position.x = client->session_vals["warp_tox"];
    client->character->position.y = client->session_vals["warp_toy"];
    client->character->sitting = 0;

    if (client->session_vals["warp_forced"] == 0 || client->session_vals["warp_forced_p2"] > 0)
    {
        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WARP, eoencoding::PACTION_AGREE);
        client->get_packet_builder()->add_int(2, 1);
        client->get_packet_builder()->add_int(map, 2);
        client->get_packet_builder()->add_int(client->session_vals["warp_anim"], 1);

        uint8_t players = 0;
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && client->character->in_range(c->character))
            {
                ++players;
            }
        }
        client->get_packet_builder()->add_int(players, 1);
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && client->character->in_range(c->character))
            {
                add_player_to_packet_builder(server, client, c.get());
            }
        }
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        // npcs here
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        add_item_drops_in_range_to_packet_builder(server, client);
        client->Send();
    }

    if (client->session_vals["warp_forced"] == 0 || client->session_vals["warp_forced_p2"] == 0)
    {
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && client->character->in_range(c->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_AGREE);
                add_player_to_packet_builder(server, c.get(), client);
                c->get_packet_builder()->add_int(client->session_vals["warp_anim"], 1);
                c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                c->get_packet_builder()->add_int(1, 1);
                c->Send();
            }
        }
    }

    if (client->session_vals["warp_forced"] > 0 && client->session_vals["warp_forced_p2"] == 0)
    {
        client->session_vals["warp_forced_p2"] = 1;
        return;
    }

    client->session_vals["warp_secid"] = 0;
    client->session_vals["warp_tomap"] = 0;
    client->session_vals["warp_tox"] = 0;
    client->session_vals["warp_toy"] = 0;
    client->session_vals["warp_anim"] = 0;
    client->session_vals["nirvana"] = 0;
    client->session_vals["warp_forced_p2"] = 0;
}
