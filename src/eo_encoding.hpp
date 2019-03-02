/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __EO_ENCODING_HPP_DEFINED
#define __EO_ENCODING_HPP_DEFINED

#include <string>
#include <vector>

#include "output.hpp"

#define EO_INT2 253
#define EO_INT3 64009
#define EO_INT4 16194277
#define EO_MAXINT 4097152080

namespace eoencoding
{
    enum PFamily
    {
        PFAMILY_CONNECTION = 1,
        PFAMILY_ACCOUNT = 2,
        PFAMILY_CHARACTER = 3,
        PFAMILY_LOGIN = 4,
        PFAMILY_WELCOME = 5,
        PFAMILY_WALK = 6,
        PFAMILY_FACE = 7,
        PFAMILY_CHAIR = 8,
        PFAMILY_EMOTE = 9,
        PFAMILY_ATTACK = 11,
        PFAMILY_SPELL = 12,
        PFAMILY_SHOP = 13,
        PFAMILY_ITEM = 14,
        PFAMILY_STATSKILL = 16,
        PFAMILY_GLOBAL = 17,
        PFAMILY_TALK = 18,
        PFAMILY_WARP = 19,
        PFAMILY_JUKEBOX = 21,
        PFAMILY_PLAYERS = 22,
        PFAMILY_AVATAR = 23,
        PFAMILY_PARTY = 24,
        PFAMILY_REFRESH = 25,
        PFAMILY_NPC = 26,
        PFAMILY_AUTOREFRESH = 27,
        PFAMILY_NPCAUTOREFRESH = 28,
        PFAMILY_APPEAR = 29,
        PFAMILY_PAPERDOLL = 30,
        PFAMILY_EFFECT = 31,
        PFAMILY_TRADE = 32,
        PFAMILY_CHEST = 33,
        PFAMILY_DOOR = 34,
        PFAMILY_MESSAGE = 35,
        PFAMILY_BANK = 36,
        PFAMILY_LOCKER = 37,
        PFAMILY_BARBER = 38,
        PFAMILY_GUILD = 39,
        PFAMILY_SOUND = 40,
        PFAMILY_SIT = 41,
        PFAMILY_RECOVER = 42,
        PFAMILY_BOARD = 43,
        PFAMILY_ARENA = 45,
        PFAMILY_PRIEST = 46,
        PFAMILY_MARRIAGE = 47,
        PFAMILY_ADMINITERACT = 48,
        PFAMILY_CITIZEN = 49,
        PFAMILY_QUEST = 50,
        PFAMILY_BOOK = 51,
        PFAMILY_INIT = 255
    };

    enum PAction
    {
        PACTION_REQUEST = 1,
        PACTION_ACCEPT = 2,
        PACTION_REPLY = 3,
        PACTION_REMOVE = 4,
        PACTION_AGREE = 5,
        PACTION_CREATE = 6,
        PACTION_ADD = 7,
        PACTION_PLAYER = 8,
        PACTION_TAKE = 9,
        PACTION_USE = 10,
        PACTION_BUY = 11,
        PACTION_SELL = 12,
        PACTION_OPEN = 13,
        PACTION_CLOSE = 14,
        PACTION_MESSAGE = 15,
        PACTION_SPEC = 16,
        PACTION_ADMIN = 17,
        PACTION_LIST = 18,
        PACTION_TELL = 20,
        PACTION_REPORT = 21,
        PACTION_ANNOUNCE = 22,
        PACTION_SERVER = 23,
        PACTION_DROP = 24,
        PACTION_JUNK = 25,
        PACTION_GET = 27,
        PACTION_TARGETNPC = 31,
        PACTION_EXP = 33,
        PACTION_DIALOG = 34,
        PACTION_PING = 240,
        PACTION_PONG = 241,
        PACTION_NET3 = 242,
        PACTION_INIT = 255
    };

    enum EOByte
    {
        BYTE_NULL = 254,
        BYTE_COMMA = 255
    };

    enum PacketType
    {
        PACKET_RAW,
        PACKET_ENCODED
    };

    unsigned char get_eobyte(EOByte byte);

    std::string string_interleave(std::string str);
    std::string string_deinterleave(std::string str);
    std::string xor128(std::string str);
    std::string dwind(std::string str, uint8_t multi);
    std::vector<uint8_t> eoint_encode(unsigned int n, uint8_t len = 0);
    unsigned int eoint_decode(std::vector<uint8_t> eoint);
};

#endif // __EO_ENCODING_HPP_DEFINED
