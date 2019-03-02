/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __HANDLERS_HPP_INCLUDED
#define __HANDLERS_HPP_INCLUDED

#include <memory>
#include <map>
#include <queue>
#include <algorithm>

#include "../util.hpp"
#include "../output.hpp"
#include "../server.hpp"
#include "../client.hpp"
#include "../character.hpp"
#include "../events/events.hpp"

#define REGISTER_HANDLER(client, f, a, handler) client->get_handler_manager()->Register(std::make_pair(f, a), handler);
#define UNREGISTER_HANDLER(client, f, a) client->get_handler_manager()->Unregister(std::make_pair(f, a));

class Server;
class EOClient;

namespace handler
{
    void init(Server *server, EOClient *client);
    void init_ack(Server *server, EOClient *client);
    void pong(Server *server, EOClient *client);
    void login(Server *server, EOClient *client);
    void name_availability(Server *server, EOClient *client);
    void register_account(Server *server, EOClient *client);
    void change_password(Server *server, EOClient *client);
    void request_create_character(Server *server, EOClient *client);
    void create_character(Server *server, EOClient *client);
    void request_delete_character(Server *server, EOClient *client);
    void delete_character(Server *server, EOClient *client);
    void request_welcome(Server *server, EOClient *client);
    void send_file(Server *server, EOClient *client);
    void welcome(Server *server, EOClient *client);
    void online_list(Server *server, EOClient *client);
    void online_list_friends(Server *server, EOClient *client);
    void find_player(Server *server, EOClient *client);
    void latency(Server *server, EOClient *client);
    void scr_chat(Server *server, EOClient *client);
    void prv_chat(Server *server, EOClient *client);
    void guild_chat(Server *server, EOClient *client);
    void adm_chat(Server *server, EOClient *client);
    void glb_chat(Server *server, EOClient *client);
    void glb_open(Server *server, EOClient *client);
    void glb_close(Server *server, EOClient *client);
    void anc_chat(Server *server, EOClient *client);
    void emote(Server *server, EOClient *client);
    void face(Server *server, EOClient *client);
    void sit(Server *server, EOClient *client);
    void chair(Server *server, EOClient *client);
    void walk(Server *server, EOClient *client);
    void refresh(Server *server, EOClient *client);
    void appear_approach(Server *server, EOClient *client);
    void attack(Server *server, EOClient *client);
    void open_paperdoll(Server *server, EOClient *client);
    void open_book(Server *server, EOClient *client);
    void equip(Server *server, EOClient *client);
    void unequip(Server *server, EOClient *client);
    void junk(Server *server, EOClient *client);
    void drop(Server *server, EOClient *client);
    void pick(Server *server, EOClient *client);
    void warp_nomap(Server *server, EOClient *client);
    void warp(Server *server, EOClient *client);
    void door(Server *server, EOClient *client);
    void open_questlist(Server *server, EOClient *client);
    void take_quest(Server *server, EOClient *client);
    void shop_buy(Server *server, EOClient *client);
    void shop_sell(Server *server, EOClient *client);
};

#endif // __HANDLERS_HPP_INCLUDED
