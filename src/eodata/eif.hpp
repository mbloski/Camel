/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __EIF_HPP_INCLUDED
#define __EIF_HPP_INCLUDED

#include <string>
#include <map>

#include "eodata.hpp"
#include "../output.hpp"

namespace eodata
{
    class EIF
    {
        public:
        enum item_type
        {
            TYPE_STATIC,
            TYPE_STATICVAL,
            TYPE_CURRENCY,
            TYPE_POTION,
            TYPE_SCROLL,
            TYPE_SPELL,
            TYPE_EXP_REWARD,
            TYPE_STAT_REWARD,
            TYPE_SKILL_REWARD,
            TYPE_KEY,
            TYPE_WEAPON,
            TYPE_SHIELD,
            TYPE_ARMOR,
            TYPE_HAT,
            TYPE_SHOES,
            TYPE_GLOVES,
            TYPE_MISC,
            TYPE_BELT,
            TYPE_NECKLACE,
            TYPE_RING,
            TYPE_ARMLET,
            TYPE_BRACER,
            TYPE_ALCOHOL,
            TYPE_EFFECT,
            TYPE_HAIR_DYE,
            TYPE_OTHER_POTION,
            TYPE_UNKNOWN1,
            TYPE_UNKNOWN2,
            TYPE_UNKNOWN3,
            TYPE_UNKNOWN4,
            TYPE_UNKNOWN5
        };

        enum item_subtype
        {
            TYPE_NORMAL,
            TYPE_RANGED,
            TYPE_AMMO,
            TYPE_WINGS
        };

        enum item_rarity
        {
            RARITY_COMMON,
            RARITY_UNCOMMON,
            RARITY_RARE,
            RARITY_RAREST,
            RARITY_LORE,
            RARITY_CURSED,
            RARITY_UNKNOWN1,
            RARITY_UNKNOWN2
        };

        enum item_size
        {
            SIZE_1x1,
            SIZE_1x2,
            SIZE_1x3,
            SIZE_1x4,
            SIZE_2x1,
            SIZE_2x2,
            SIZE_2x3,
            SIZE_2x4
        };

        struct item_t
        {
            std::string name;
            uint16_t gfx = 0;
            item_type type = TYPE_STATIC;
            item_subtype subtype = TYPE_NORMAL;
            item_rarity rarity = RARITY_COMMON;
            uint16_t hp = 0;
            uint16_t tp = 0;

            struct
            {
                uint16_t min = 0;
                uint16_t max = 0;
            } damage;

            uint16_t accuracy = 0;
            uint16_t evasion = 0;
            uint16_t defense = 0;

            struct
            {
                uint8_t str = 0;
                uint8_t intelligence = 0;
                uint8_t wis = 0;
                uint8_t agi = 0;
                uint8_t con = 0;
                uint8_t cha = 0;
            } stats;

            struct
            {
                uint8_t light = 0;
                uint8_t dark = 0;
                uint8_t earth = 0;
                uint8_t air = 0;
                uint8_t water = 0;
                uint8_t fire = 0;
            } resistance;

            struct
            {
                uint32_t map = 0;
                uint8_t x = 0;
                uint8_t y = 0;
            } warp;

            uint32_t look = 0;
            uint8_t gender = 0;

            uint16_t level_requirement = 0;
            uint16_t class_requirement = 0;

            struct
            {
                uint16_t str = 0;
                uint16_t intelligence = 0;
                uint16_t wis = 0;
                uint16_t agi = 0;
                uint16_t con = 0;
                uint16_t cha = 0;
            } stats_requirements;

            uint8_t weight = 0;
            item_size size = SIZE_1x1;

        };

        struct shop_item_t
        {
            uint16_t id;
            size_t buy_price;
            size_t sell_price;
            uint16_t max_amount = 4;
        };

        EIF();
        EIF(const std::string &filename);
        void operator=(const std::string &filename);
        std::string get_filename() const { return this->filename; }
        std::string get_checksum() const { return this->checksum; }
        std::string get_file() const { return this->raw_file; }
        item_t get(size_t id);

        private:
        size_t parse();
        std::string raw_file;
        std::string filename;
        std::string checksum;
        std::map<size_t, item_t> items;

    };
}

#endif // __EIF_HPP_INCLUDED
