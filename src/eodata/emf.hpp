/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __EMF_HPP_INCLUDED
#define __EMF_HPP_INCLUDED

#include <string>
#include <sstream>
#include <iomanip>
#include <map>

#include "eodata.hpp"

namespace eodata
{
    class EMF
    {
        public:
        enum tile_t
        {
            TILE_NORMAL = -1,
            TILE_WALL,
            TILE_CHAIR_DOWN,
            TILE_CHAIR_LEFT,
            TILE_CHAIR_RIGHT,
            TILE_CHAIR_UP,
            TILE_CHAIR_DOWN_RIGHT,
            TILE_CHAIR_UP_LEFT,
            TILE_CHAIR_ALL,
            TILE_DOOR,
            TILE_CHEST,
            TILE_UNKNOWN1,
            TILE_UNKNOWN2,
            TILE_ARENA, // not used on EOMain
            TILE_UNKNOWN4,
            TILE_UNKNOWN5,
            TILE_UNKNOWN6,
            TILE_BANK,
            TILE_NPCWALL,
            TILE_EDGE,
            TILE_FAKEWALL,
            TILE_BOARD1,
            TILE_BOARD2,
            TILE_BOARD3,
            TILE_BOARD4,
            TILE_BOARD5,
            TILE_BOARD6,
            TILE_BOARD7,
            TILE_BOARD8,
            TILE_JUKEBOX,
            TILE_JUMP,
            TILE_WATER,
            TILE_UNKNOWN7,
            TILE_NOGHOST,
            TILE_UNKNOWN8,
            TILE_SPIKES1,
            TILE_SPIKES2,
            TILE_SPIKES3
        };

        enum warp_t
        {
            WARP_NORMAL,
            WARP_DOOR,
            WARP_SILVER_KEY,
            WARP_CRYSTAL_KEY,
            WARP_WRAITH_KEY
        };

        struct drop_t
        {
            std::pair<uint8_t, uint8_t> pos;
            uint16_t id;
            size_t amount = 1;
            uint8_t effect = 0;
            std::time_t time = std::time(0);
            uint16_t pid = 0;
            bool archived = false;
        };

        struct map_t
        {
            std::string path;
            std::string checksum;

            size_t id = 0;
            std::string name = "";
            bool pk = false;

            uint8_t width;
            uint8_t height;

            bool scrollable;
            uint8_t relogx;
            uint8_t relogy;

            struct warp
            {
                uint16_t map = 0;
                uint8_t x = 0;
                uint8_t y = 0;
                uint8_t level;
                warp_t type;
                bool open;

                bool exists()
                {
                    if (map && x && y)
                    {
                        return true;
                    }

                    return false;
                }
            };

            struct npc
            {
                uint16_t id;
                uint8_t type;
                uint16_t time;
                uint8_t amount;
            };

            struct chest_item
            {
                uint16_t id;
                uint8_t slot;
                uint16_t time;
                uint32_t amount;
            };

            struct chest
            {
                uint16_t key;
                std::vector<chest_item> items;
            };

            struct tile
            {
                tile_t type = TILE_NORMAL;
                struct warp warp;
                struct npc npc;
                struct chest chest;
            };

            std::map<std::pair<uint8_t, uint8_t>, tile> tiles;
            std::vector<std::pair<size_t, drop_t>> drops;
            bool is_walkable(uint8_t x, uint8_t y);
        };

        private:
        size_t mapcount;
        std::string directory;
        std::map<size_t, map_t> maps;

        public:
        EMF(size_t howmany, const std::string &directory);
        void operator=(const std::string &directory);
        size_t parse(size_t id);
        bool add_item(size_t id, drop_t drop);
        bool archive_item(size_t id, size_t item_id);
        bool update(size_t id, map_t newmap);
        struct map_t get(size_t id);
    };
};

#endif // __EMF_HPP_INCLUDED
