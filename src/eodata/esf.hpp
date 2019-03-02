/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __ESF_HPP_INCLUDED
#define __ESF_HPP_INCLUDED

#include <string>
#include <map>

#include "eodata.hpp"
#include "../output.hpp"

namespace eodata
{
    class ESF
    {
        private:
        size_t parse();
        std::string raw_file;
        std::string filename;
        std::string checksum;

        enum spell_type
        {
            TYPE_HEAL,
            TYPE_OFFENSIVE,
            TYPE_BARD
        };

        enum spell_purpose
        {
            PURPOSE_NPC,
            PURPOSE_FRIEND,
            PURPOSE_ENEMY
        };

        enum spell_target
        {
            TARGET_NORMAL,
            TARGET_SELF,
            TARGET_UNKNOWN1,
            TARGET_GROUP
        };

        struct spell_t
        {
            std::string name;
            std::string invocation;
            uint16_t icon = 0;
            uint16_t gfx = 0;
            uint16_t tp = 0;
            uint16_t sp = 0;
            uint8_t casttime = 0;
            spell_type type = TYPE_HEAL;
            spell_purpose purpose = PURPOSE_NPC;
            spell_target target = TARGET_NORMAL;

            struct
            {
                uint16_t min = 0;
                uint16_t max = 0;
            } damage;

            uint16_t accuracy = 0;
            uint16_t hp = 0;
        };

        std::map<size_t, spell_t> spells;

        public:
        ESF();
        ESF(const std::string &filename);
        void operator=(const std::string &filename);
        std::string get_filename() const { return this->filename; }
        std::string get_checksum() const { return this->checksum; }
        std::string get_file() const { return this->raw_file; }
        spell_t get(size_t id);
    };
}

#endif // __ESF_HPP_INCLUDED

