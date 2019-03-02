/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __ECF_HPP_INCLUDED
#define __ECF_HPP_INCLUDED

#include <string>
#include <map>

#include "eodata.hpp"
#include "../output.hpp"

namespace eodata
{
    class ECF
    {
        private:
        size_t parse();
        std::string raw_file;
        std::string filename;
        std::string checksum;

        struct class_t
        {
            size_t id = 0;
            std::string name;
            uint8_t base = 0;
            uint8_t type = 0;

            struct
            {
                uint16_t str = 0;
                uint16_t intelligence = 0;
                uint16_t wis = 0;
                uint16_t agi = 0;
                uint16_t con = 0;
                uint16_t cha = 0;
            } stats;
        };

        std::map<size_t, class_t> classes;

        public:
        ECF();
        ECF(const std::string &filename);
        void operator=(const std::string &filename);
        std::string get_filename() const { return this->filename; }
        std::string get_checksum() const { return this->checksum; }
        std::string get_file() const { return this->raw_file; }
        class_t get(size_t id);
    };
}

#endif // __ECF_HPP_INCLUDED
