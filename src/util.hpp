/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __UTIL_HPP_INCLUDED
#define __UTIL_HPP_INCLUDED

#include <sys/time.h>
#include <time.h>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <random>
#include <openssl/sha.h>
#include <cmath>

#include "output.hpp"

namespace util
{
    struct tm get_localtime();
    size_t get_milliseconds();
    size_t get_eo_timestamp();
    std::vector<std::string> tokenize(std::string str, char separator);

    std::string rtrim(std::string str);
    std::string ltrim(std::string str);
    std::string trim(std::string str);

    enum str_pad_mode
    {
        STR_PAD_RIGHT,
        STR_PAD_LEFT,
        STR_PAD_BOTH
    };

    std::string str_pad(std::string str, size_t w, char fill = ' ', str_pad_mode mode = STR_PAD_RIGHT);
    std::string strtolower(std::string str);
    std::string strtoupper(std::string str);
    bool isalpha(std::string str);
    bool isplaintext(std::string str);
    bool isnumeric(std::string str);

    std::string rand_string(size_t length);
    std::string prettybytes(std::string str);

    std::string sha256(std::string str);

    class rand
    {
        public:
        rand();
        template<typename T = int> T get_int(int _min, int _max)
        {
            std::uniform_int_distribution<T> dist(_min, _max);
            return dist(this->engine);
        }

        private:
        std::mt19937 engine;
    };

    long int get_tnl(uint8_t level, size_t exp);
}

#endif // __UTIL_HPP_INCLUDED
