/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __OUTPUT_HPP_INCLUDED
#define __OUTPUT_HPP_INCLUDED

#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <iomanip>

#include "util.hpp"

#define PRE_APPEND_TIMESTAMP()\
struct tm time = util::get_localtime();\
pre << std::setfill('0') << std::setw(2) << time.tm_hour\
<< ":"\
<< std::setfill('0') << std::setw(2) << time.tm_min\
<< ":"\
<< std::setfill('0') << std::setw(2) << time.tm_sec;\

#define CONSOLE_OUTPUT_GEN(stream, pre)\
do {\
    va_list args;\
    va_start(args, message);\
    vfprintf(stream == STREAM_OUT? stdout : stderr, (std::string(pre) + std::string(message) + "\n").c_str(), args);\
    va_end(args);\
} while (false)

namespace out
{
    extern bool colorful;

    enum Stream
    {
        STREAM_OUT,
        STREAM_ERR
    };

    enum Color
    {
        COLOR_BLACK,
        COLOR_GREEN,
        COLOR_RED,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_PURPLE,
        COLOR_CYAN,
        COLOR_WHITE,
        COLOR_BLACK_BACKGROUND,
        COLOR_GREEN_BACKGROUND,
        COLOR_RED_BACKGROUND,
        COLOR_YELLOW_BACKGROUND,
        COLOR_BLUE_BACKGROUND,
        COLOR_PURPLE_BACKGROUND,
        COLOR_CYAN_BACKGROUND,
        COLOR_WHITE_BACKGROUND,
        COLOR_RESET
    };

    inline std::string build_color(Color color, bool bold = false);
    std::string colorize(std::string str, Color color, bool bold = false);

    void info(const char *message, ...);
    void error(const char *message, ...);
    void warning(const char *message, ...);
#ifdef DEBUG
    void debug(const char *message, ...);
#endif // DEBUG
    void gen(const char *message, ...);
};

#endif // __OUTPUT_HPP_INCLUDED
