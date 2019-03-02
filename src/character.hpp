/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __CHARACTER_HPP_INCLUDED
#define __CHARACTER_HPP_INCLUDED

#include "database.hpp"
#include "util.hpp"

class Inventory
{
    private:
    std::map<size_t, size_t> inventory;

    public:
    Inventory(std::string str);
    void add(size_t id, size_t amount = 1);
    void remove(size_t id, size_t amount = 1);
    size_t get_amount(size_t id);
    std::map<size_t, size_t> get() const { return inventory; }
    std::string get_serialized();
};

class Paperdoll
{
    public:
    Paperdoll(std::string str);
    std::string get_serialized();
    uint16_t armor = 0;
    uint16_t hat = 0;
    uint16_t boots = 0;
    uint16_t weapon = 0;
    uint16_t shield = 0;
    uint16_t gloves = 0;
    uint16_t belt = 0;
    uint16_t necklace = 0;
    std::pair<uint16_t, uint16_t> rings = std::make_pair(0, 0);
    std::pair<uint16_t, uint16_t> bracelets = std::make_pair(0, 0);
    std::pair<uint16_t, uint16_t> bracers = std::make_pair(0, 0);
    uint8_t gem = 0;
};

struct Character
{
    public:
    Character(std::shared_ptr<sql::ResultSet> sqldata);
    ~Character();

    void UpdateStats();
    bool in_range(Character *character, size_t distance = 15);
    bool in_range_of(size_t map, uint8_t x, uint8_t y, size_t distance = 15);

    size_t id;
    std::string name = "";
    uint8_t level;
    uint8_t gender;
    uint8_t race;
    uint8_t hairstyle;
    uint8_t haircolor;
    std::string home = "";
    std::string partner = "";
    std::string title = "";
    size_t exp;
    uint8_t admin;

    struct
    {
        uint16_t map;
        uint8_t x;
        uint8_t y;
        uint8_t direction;
    } position;

    uint8_t sitting;
    size_t hp;
    size_t tp;
    size_t class_id;

    struct
    {
        uint16_t str;
        uint16_t intelligence;
        uint16_t wis;
        uint16_t agi;
        uint16_t con;
        uint16_t cha;
    } stats;

    bool hidden;
    size_t karma;
    size_t usage;
    size_t arena_wins;
    size_t arena_kills;
    size_t arena_deaths;

    time_t online_since = std::time(0);

    std::string guild = "";
    uint8_t guild_rank;
    std::string guild_rank_str = "";

    /* This is a session value and does not go back to the database. */
    std::string guild_name = "";

    Inventory *inventory;
    Paperdoll *paperdoll;
};

#endif // __CHARACTER_HPP_INCLUDED
