/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __EODATA_HPP_INCLUDED
#define __EODATA_HPP_INCLUDED

#include <fstream>
#include <sstream>
#include <errno.h>
#include <cstring>

#include "../eo_encoding.hpp"
#include "../packet_reader.hpp"

namespace eodata
{
    inline int raise_data_error(std::string filename, std::string error)
    {
        out::error("There was an error while reading a data file:");
        out::error("^ \"%s\": %s", out::colorize(filename, out::COLOR_WHITE).c_str(), error.c_str());
        return 0xDA;
    }

    inline std::string get_file(std::string filename)
    {
        std::string ret;
        std::ifstream file(filename, std::ios::in | std::ios::binary);
        if (file)
        {
            ret = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        }
        else
        {
            exit(eodata::raise_data_error(filename, strerror(errno)));
        }

        return ret;
    }
}

#endif // __EODATA_HPP_INCLUDED

