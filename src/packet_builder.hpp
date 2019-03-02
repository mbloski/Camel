/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __PACKET_BUILDER_HPP_INCLUDED
#define __PACKET_BUILDER_HPP_INCLUDED

#include <string>

#include "output.hpp"
#include "eo_encoding.hpp"

class PacketBuilder
{
    private:
    uint8_t multi;
    eoencoding::PacketType packet_mode;
    std::string buffer;

    public:
    PacketBuilder(uint8_t multi);
    uint8_t get_multi() const { return this->multi; }
    enum eoencoding::PacketType get_packet_mode() const { return this->packet_mode; }
    void PreparePacket(eoencoding::PFamily family, eoencoding::PAction action);

    void add_int(unsigned int n, size_t length = 0);
    void add_char(unsigned char c);
    void add_string(std::string str, bool with_comma = false);

    std::string get();
};

#endif // __PACKET_BUILDER_HPP_INCLUDED
