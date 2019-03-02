/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __PACKET_READER_HPP_INCLUDED
#define __PACKET_READER_HPP_INCLUDED

#include <string>

#include "output.hpp"
#include "eo_encoding.hpp"

class PacketReader
{
    private:
    uint8_t multi;
    eoencoding::PacketType packet_mode;
    std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type;

    std::string buffer;
    size_t pos = 0;
    std::string fullpacket;

    uint8_t sequence;
    uint8_t received_sequence;
    uint8_t sequence_counter = 0;

    public:
    PacketReader(uint8_t multi);
    void Load(std::string str, bool noencode = false);
    inline void Clear();
    void SetSequence(uint8_t seq);

    uint8_t get_multi() const { return this->multi; }
    enum eoencoding::PacketType get_packet_mode() const { return this->packet_mode; }
    uint8_t get_sequence() const { return this->sequence + this->sequence_counter; }
    uint8_t get_received_sequence() const { return this->received_sequence; }
    size_t get_pos() const { return this->pos; }

    bool is_end();
    unsigned int get_int(size_t length);
    unsigned char get_char();
    std::string get_string();
    std::string get_string(size_t length);

    void increase_sequence_counter();

    std::string get() const
    {
        return this->fullpacket;
    }

    eoencoding::PFamily get_family() const
    {
        return this->packet_type.first;
    }

    eoencoding::PAction get_action() const
    {
        return this->packet_type.second;
    }

    size_t bytes_left();
};

#endif // __PACKET_READER_HPP_INCLUDED
