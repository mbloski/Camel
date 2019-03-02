/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __ENF_HPP_INCLUDED
#define __ENF_HPP_INCLUDED

#include <string>
#include <map>

#include "eodata.hpp"
#include "../output.hpp"

namespace eodata
{
    class ENF
    {
        private:
        size_t parse();
        std::string raw_file;
        std::string filename;
        std::string checksum;

        enum npc_type
        {
            TYPE_NPC,
            TYPE_PASSIVE,
            TYPE_AGGRESSIVE,
            TYPE_SHOP = 6,
            TYPE_INN,
            TYPE_BANK = 9,
            TYPE_BARBER,
            TYPE_GUILD,
            TYPE_PRIEST,
            TYPE_LAW,
            TYPE_SKILLS,
            TYPE_QUEST
        };

        struct npc_t
        {
            std::string name;
            uint16_t gfx = 0;
            uint16_t boss = 0;
            uint16_t child = 0;
            npc_type type = TYPE_NPC;
            uint16_t id = 0;
            uint32_t hp = 0;

            struct
            {
                uint16_t min = 0;
                uint16_t max = 0;
            } damage;

            uint16_t accuracy = 0;
            uint16_t evasion = 0;
            uint16_t defense = 0;
            uint32_t exp = 0;
        };

        std::map<size_t, npc_t> npcs;

        public:
        ENF();
        ENF(const std::string &filename);
        void operator=(const std::string &filename);
        std::string get_filename() const { return this->filename; }
        std::string get_checksum() const { return this->checksum; }
        std::string get_file() const { return this->raw_file; }
        npc_t get(size_t id);
    };
}

#endif // __ENF_HPP_INCLUDED

