/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "packet_builder.hpp"

PacketBuilder::PacketBuilder(uint8_t multi)
{
    this->multi = multi;
}

void PacketBuilder::PreparePacket(eoencoding::PFamily family, eoencoding::PAction action)
{
    this->buffer.clear();
    this->buffer = static_cast<unsigned char>(action);
    this->buffer += static_cast<unsigned char>(family);

    if (family == eoencoding::PFAMILY_INIT && action == eoencoding::PACTION_INIT)
    {
        /* 255_255 - we're dealing with an unencrypted packet. */
        this->packet_mode = eoencoding::PACKET_RAW;
    }
    else
    {
        this->packet_mode = eoencoding::PACKET_ENCODED;
    }
}

void PacketBuilder::add_int(unsigned int n, size_t length)
{
    std::vector<uint8_t> encoded_number = eoencoding::eoint_encode(n, length);
    this->buffer += std::string(encoded_number.begin(), encoded_number.end());
}

void PacketBuilder::add_char(unsigned char c)
{
    this->buffer += c;
}

void PacketBuilder::add_string(std::string str, bool with_comma)
{
    if (with_comma)
    {
        this->buffer += eoencoding::get_eobyte(eoencoding::BYTE_COMMA);
    }
    this->buffer += str;
}

std::string PacketBuilder::get()
{
    std::vector<uint8_t> len = eoencoding::eoint_encode(this->buffer.length(), 2);

    if (this->packet_mode == eoencoding::PACKET_ENCODED)
    {
        this->buffer = eoencoding::string_interleave(eoencoding::xor128(eoencoding::dwind(this->buffer, this->multi)));
    }

    return std::string(len.begin(), len.end()) + this->buffer;
}
