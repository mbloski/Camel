/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "output.hpp"

bool out::colorful = true;

/**
    Builds and returns a VT100-compatible color escape sequence.
*/
inline std::string out::build_color(Color color, bool bold)
{
    std::string buffer = "\033[";
    std::string color_code;
    switch (color)
    {
        case COLOR_BLACK:              color_code = "30"; break;
        case COLOR_RED:                color_code = "31"; break;
        case COLOR_GREEN:              color_code = "32"; break;
        case COLOR_YELLOW:             color_code = "33"; break;
        case COLOR_BLUE:               color_code = "34"; break;
        case COLOR_PURPLE:             color_code = "35"; break;
        case COLOR_CYAN:               color_code = "36"; break;
        case COLOR_WHITE:              color_code = "97"; break; // 37
        case COLOR_BLACK_BACKGROUND:   color_code = "40"; break;
        case COLOR_RED_BACKGROUND:     color_code = "41"; break;
        case COLOR_GREEN_BACKGROUND:   color_code = "42"; break;
        case COLOR_YELLOW_BACKGROUND:  color_code = "43"; break;
        case COLOR_BLUE_BACKGROUND:    color_code = "44"; break;
        case COLOR_PURPLE_BACKGROUND:  color_code = "45"; break;
        case COLOR_CYAN_BACKGROUND:    color_code = "46"; break;
        case COLOR_WHITE_BACKGROUND:   color_code = "47"; break;

        case COLOR_RESET:
            /* If it's a "reset" color, append the remaining bytes and return immediately. */
            buffer += "0m";
            return buffer;
        break;
    }

    buffer += bold? "1" : "0";
    buffer += ";";
    buffer += color_code;
    buffer += "m";
    return buffer;
}

std::string out::colorize(std::string str, Color color, bool bold)
{
    std::string buffer;
    if (colorful)
    {
        buffer = build_color(color, bold) + str + build_color(COLOR_RESET);
    }
    else
    {
        /* In case colors are disabled, simply return the boring, unmodified string. */
        buffer = str;
    }
    return buffer;
}

void out::info(const char *message, ...)
{
    std::stringstream pre;
    PRE_APPEND_TIMESTAMP();
    pre << " " << colorize(colorful? "*" : "i", COLOR_GREEN, true) << "  ";
    CONSOLE_OUTPUT_GEN(STREAM_OUT, pre.str());
}

void out::error(const char *message, ...)
{
    std::stringstream pre;
    PRE_APPEND_TIMESTAMP();
    pre << " " << colorize(colorful? "*" : "E", COLOR_RED, true) << "  ";
    CONSOLE_OUTPUT_GEN(STREAM_ERR, pre.str());
}

void out::warning(const char *message, ...)
{
    std::stringstream pre;
    PRE_APPEND_TIMESTAMP();
    pre << " " << colorize(colorful? "*" : "!", COLOR_YELLOW, true) << "  ";
    CONSOLE_OUTPUT_GEN(STREAM_OUT, pre.str());
}

#ifdef DEBUG
void out::debug(const char *message, ...)
{
    std::stringstream pre;
    PRE_APPEND_TIMESTAMP();
    pre << " " << colorize(colorful? "*" : "D", COLOR_PURPLE, true) << "  ";
    CONSOLE_OUTPUT_GEN(STREAM_OUT, pre.str());
}
#endif // DEBUG

void out::gen(const char *message, ...)
{
    std::stringstream pre;
    PRE_APPEND_TIMESTAMP();
    pre << " ";
    CONSOLE_OUTPUT_GEN(STREAM_OUT, pre.str());
}
