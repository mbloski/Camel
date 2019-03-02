/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "packet_reader.hpp"

PacketReader::PacketReader(uint8_t multi)
{
    this->multi = multi;
}

void PacketReader::Load(std::string str, bool noencode)
{
    this->pos = 0;

    if (str.length() < 3)
    {
        this->Clear();
        return;
    }

    int startpos;

    eoencoding::PFamily family = (eoencoding::PFamily)(uint8_t)str[1];
    eoencoding::PAction action = (eoencoding::PAction)(uint8_t)str[0];

    if (noencode || (family == eoencoding::PFAMILY_INIT && action == eoencoding::PACTION_INIT) || this->multi == 0)
    {
        this->packet_mode = eoencoding::PACKET_RAW;
        if (this->multi == 0)
        {
            /* This makes it convenient to read data files such as EMF or EIF with PacketReader.
               Also makes it unable to read usual packets thus 0 should never be set as a default EOClient's recv multi value. */
            startpos = 0;
        }
        else
        {
            startpos = 2;
        }
    }
    else
    {
        this->packet_mode = eoencoding::PACKET_ENCODED;
        str = eoencoding::dwind(eoencoding::xor128(eoencoding::string_deinterleave(str)), this->multi);
        family = (eoencoding::PFamily)(uint8_t)str[1];
        action = (eoencoding::PAction)(uint8_t)str[0];
        this->received_sequence = eoencoding::eoint_decode(std::vector<uint8_t>{(uint8_t)str[2]});
        startpos = 3;
    }

    this->packet_type = std::make_pair(family, action);
    this->fullpacket = str;
    this->buffer = this->fullpacket.substr(startpos, -1);
}

inline void PacketReader::Clear()
{
    this->fullpacket.clear();
    this->buffer.clear();
    this->packet_type = std::make_pair((eoencoding::PFamily)NULL, (eoencoding::PAction)NULL);

    return;
}

void PacketReader::SetSequence(uint8_t seq)
{
    this->sequence = seq;
}

void PacketReader::increase_sequence_counter()
{
    ++this->sequence_counter;
    if (this->sequence_counter >= 10)
    {
        this->sequence_counter = 0;
    }
}

bool PacketReader::is_end()
{
    if (this->pos >= this->buffer.length())
    {
        return true;
    }

    return false;
}

unsigned int PacketReader::get_int(size_t length)
{
    if (length > 4)
    {
#ifdef DEBUG
        out::debug("PacketReader::get_int() called with a length argument of %d while the max possible EOINT size is 4.", length);
        out::debug("^ changing length to 4.");
#endif // DEBUG
        length = 4;
    }

    std::vector<uint8_t> bytes(this->buffer.begin() + this->pos, this->buffer.begin() + this->pos + length);
    this->pos += length;

#ifdef DEBUG
    if (this->pos + 2 > this->fullpacket.length())
    {
        out::debug("Packet::Reader::get_int() requested more bytes than there's left in the buffer.");
    }
#endif // DEBUG

    return eoencoding::eoint_decode(bytes);
}

unsigned char PacketReader::get_char()
{
    unsigned char ret = this->buffer[this->pos];
    this->pos += 1;
#ifdef DEBUG
    if (this->pos + 2 > this->fullpacket.length())
    {
        out::debug("Packet::Reader::get_char() requested more bytes than there's left in the buffer.");
    }
#endif // DEBUG

    return ret;
}

std::string PacketReader::get_string()
{
    std::string cut(this->buffer.begin() + this->pos, this->buffer.end());
    size_t comma_pos = cut.find(eoencoding::BYTE_COMMA);

    if (comma_pos == std::string::npos)
    {
        /* if no eoencoding::BYTE_COMMA in this->buffer, read until the end. */
        comma_pos = this->buffer.length() - this->pos;
    }

    std::string ret(this->buffer.begin() + this->pos, this->buffer.begin() + comma_pos + this->pos);
    this->pos += comma_pos + 1;

    return ret;
}

std::string PacketReader::get_string(size_t length)
{
    std::string ret(this->buffer.begin() + this->pos, this->buffer.begin() + this->pos + length);
    this->pos += length;

#ifdef DEBUG
    if (this->pos + 2 > this->fullpacket.length())
    {
        out::debug("Packet::Reader::get_string() requested more bytes than there's left in the buffer.");
    }
#endif

    return ret;
}

size_t PacketReader::bytes_left()
{
    return this->buffer.length() - this->pos;
}
