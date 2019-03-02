/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __SERVER_HPP_INCLUDED
#define __SERVER_HPP_INCLUDED

#ifdef MULTITHREADED
#include <thread>
#endif // MULTITHREADED
#include <memory>

#include "output.hpp"
#include "socket.hpp"
#include "arena.hpp"
#include "client.hpp"
#include "database.hpp"
#include "glb_buffer.hpp"
#include "schedevents.hpp"
#include "eodata/ecf.hpp"
#include "eodata/eif.hpp"
#include "eodata/enf.hpp"
#include "eodata/esf.hpp"
#include "eodata/emf.hpp"

class EOClient;
class SchedEvents;
class Arena;

class Server
{
    public:
    Server(std::shared_ptr<Config> config);
    ~Server();

    void run();

    std::vector<std::shared_ptr<EOClient>> eoclients;
    std::vector<std::shared_ptr<Arena>> arenas;
    std::map<size_t, std::pair<std::string, std::vector<eodata::EIF::shop_item_t>>> shops;
    std::string eo_version;
    size_t max_chat_message_length = 128;
    bool accept_new_accounts = true;
    bool accept_logins = true;
    bool accept_characters = true;
    size_t init_patience = 10;
    size_t max_players = 1000;
    size_t jail_map = 76;
    size_t arena_kill_exp = 2;
    size_t arena_win_exp = 5;
    size_t arena_kill_gold = 3;
    size_t arena_win_gold = 5;
    std::list<std::pair<size_t, size_t>> arena_class_hierarchy;
    size_t idle_gold = 1;
    size_t idle_gold_time = 5;
    bool shop_discounts = false;
    size_t shop_discount_threshold = 30000;
    size_t shop_max_discount = 80;
    unsigned char admin_prefix = '$';
    size_t database_autoupdate_delay = 0;
    size_t glb_buffer_size = 7;

    std::list<uint16_t> id_cache;

    struct
    {
        eodata::ECF classes;
        eodata::EIF items;
        eodata::ENF npcs;
        eodata::ESF spells;
    } pub;

    eodata::EMF *maps;

    std::vector<std::string> motd;
    GlbBuffer *glb_buffer = 0;

    void inc_player_count() { ++this->players_ingame; }
    void dec_player_count() { --this->players_ingame; }
    size_t get_player_count() const { return this->players_ingame; }
    Database *get_database() const { return this->database; }
    Socket *get_socket() const { return this->server; }
    bool is_running() const { return this->running; }

    util::rand random;

    SchedEvents *evman = 0;

    void WorldAnnounce(std::string str);
    void MapAnnounce(size_t id, std::string str);
    void Warp(EOClient *client, uint16_t map, uint8_t x, uint8_t y, uint8_t anim, bool forced = false);
    void AddExp(EOClient *client, size_t exp);
    void AddItem(EOClient *client, uint16_t item, size_t amount, uint16_t drop_id = 0);
    void ToggleHide(EOClient *client);
    void Shutdown();

    private:
    bool running = false;
    void HandleNewConnections();
    void CleanupConnections();
    std::shared_ptr<Config> config;
    size_t ping_timeout_threshold = 30;
    size_t players_ingame = 0;
    Database *database = 0;
    Socket *server = 0;
#ifdef MULTITHREADED
    void ConnectionHandler();
    std::thread handle_connections_thread;
#endif
};

#endif // __SERVER_HPP_INCLUDED
