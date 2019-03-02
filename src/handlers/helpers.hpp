/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __HELPERS_HPP_INCLUDED
#define __HELPERS_HPP_INCLUDED

#include "../util.hpp"
#include "../output.hpp"
#include "../server.hpp"
#include "../client.hpp"
#include "../character.hpp"

inline void add_player_to_packet_builder(Server *server, EOClient *client, EOClient *c)
{
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->name);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_int(c->id, 2);
    client->get_packet_builder()->add_int(c->character->position.map, 2);
    client->get_packet_builder()->add_int(c->character->position.x, 2);
    client->get_packet_builder()->add_int(c->character->position.y, 2);
    client->get_packet_builder()->add_int(c->character->position.direction, 1);
    client->get_packet_builder()->add_int(1, 1);
    client->get_packet_builder()->add_string(util::str_pad(c->character->guild, 3, ' ', util::STR_PAD_RIGHT));
    client->get_packet_builder()->add_int(c->character->level, 1);
    client->get_packet_builder()->add_int(c->character->gender, 1);
    client->get_packet_builder()->add_int(c->character->hairstyle, 1);
    client->get_packet_builder()->add_int(c->character->haircolor, 1);
    client->get_packet_builder()->add_int(c->character->race, 1);
    client->get_packet_builder()->add_int(c->character->hp, 2);
    client->get_packet_builder()->add_int(c->character->hp, 2); // max hp
    client->get_packet_builder()->add_int(c->character->tp, 2);
    client->get_packet_builder()->add_int(c->character->tp, 2); // max tp
    client->get_packet_builder()->add_int(server->pub.items.get(c->character->paperdoll->boots).look, 2);
    client->get_packet_builder()->add_int(0, 2); // ?
    client->get_packet_builder()->add_int(0, 2); // ?
    client->get_packet_builder()->add_int(0, 2); // ?
    client->get_packet_builder()->add_int(server->pub.items.get(c->character->paperdoll->armor).look, 2);
    client->get_packet_builder()->add_int(0, 2); // ?
    client->get_packet_builder()->add_int(server->pub.items.get(c->character->paperdoll->hat).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(c->character->paperdoll->shield).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(c->character->paperdoll->weapon).look, 2);
    client->get_packet_builder()->add_int(c->character->sitting, 1);
    client->get_packet_builder()->add_int(c->character->hidden, 1);
}

inline void add_item_drops_in_range_to_packet_builder(Server *server, EOClient *client)
{
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
            client->get_packet_builder()->add_int(drop_id, 2);
            client->get_packet_builder()->add_int(drop.id, 2);
            client->get_packet_builder()->add_int(drop.pos.first, 1);
            client->get_packet_builder()->add_int(drop.pos.second, 1);
            client->get_packet_builder()->add_int(drop.amount, 3);
        }
    }
}

inline void send_book(EOClient *client, EOClient *c)
{
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_BOOK, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_string(c->character->name);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->home);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->partner);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->title);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->guild_name);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(c->character->guild_rank_str);
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_int(c->id, 2);
    client->get_packet_builder()->add_int(c->character->class_id, 1);
    client->get_packet_builder()->add_int(c->character->gender, 1);
    client->get_packet_builder()->add_int(0, 1);
    client->get_packet_builder()->add_int(c->character->admin, 1); // TODO: party icons
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("Level: " + std::to_string(c->character->level));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("EXP: " + std::to_string(c->character->exp));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("TNL: " + std::to_string(util::get_tnl(c->character->level, c->character->exp)));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("Gold: " + std::to_string(c->character->inventory->get_amount(1)));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string(" ");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("Arena Stats");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("W: " + std::to_string(c->character->arena_wins));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("K: " + std::to_string(c->character->arena_kills));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("D: " + std::to_string(c->character->arena_deaths));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
#ifdef DEBUG
    client->get_packet_builder()->add_string(" ");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("[debug]");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_string("pid: " + std::to_string(c->id));
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
#endif // DEBUG
    client->Send();
}

#endif // __HELPERS_HPP_INCLUDED
