/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "util.hpp"

struct tm util::get_localtime()
{
    time_t t;
    struct tm *ti;

    time(&t);
    ti = localtime(&t);

    return *ti;
}

size_t util::get_milliseconds()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_usec / 1000;
}

size_t util::get_eo_timestamp()
{
    struct tm t = util::get_localtime();
    return t.tm_hour * 360000 + t.tm_min * 6000 + t.tm_sec * 100 + util::get_milliseconds() / 10;
}

std::vector<std::string> util::tokenize(std::string str, char separator)
{
    std::vector<std::string> ret;

    if (str.empty())
    {
        return ret;
    }

    size_t pos = 0;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str.substr(i, 1) == std::string(1, separator))
        {
            std::string token = str.substr(pos, i - pos);
            if (token.empty())
            {
                /* If many separators in a row, move the buffer 1 separator ahead and continue. */
                ++pos;
                continue;
            }
            ret.push_back(token);
            pos += token.length() + 1;
        }
    }
    /* Push the remaining token */
    ret.push_back(str.substr(pos));

    return ret;
}

std::string util::ltrim(std::string str)
{
    std::set<char> whitespaces = {' ', '\t', '\v', '\f', '\r', '\n'};
    size_t pos = 0;
    for (; whitespaces.find(str[pos]) != whitespaces.end() ; ++pos);

    return str.substr(pos);
}

std::string util::rtrim(std::string str)
{
    /* Just reverse the string, do ltrim and reverse again. */
    std::reverse(str.begin(), str.end());
    std::string ret = ltrim(str);
    std::reverse(ret.begin(), ret.end());

    return ret;
}

std::string util::trim(std::string str)
{
    return ltrim(rtrim(str));
}

std::string util::str_pad(std::string str, size_t w, char fill, str_pad_mode mode)
{
    std::string ret = "";
    size_t len = str.length();
    int uleft = 0, uright = 0;

    switch (mode)
    {
        case STR_PAD_RIGHT:
        uright = w - len;
        uleft = 0;
        break;
        case STR_PAD_LEFT:
        uright = 0;
        uleft = w - len;
        break;
        case STR_PAD_BOTH:
        float tpad = (w - len) / 2.0;
        uleft = std::floor(tpad);
        uright = std::ceil(tpad);
        break;
    }

    if (uleft < 0)
    {
        uleft = 0;
    }

    if (uright < 0)
    {
        uright = 0;
    }

    ret = std::string(uleft, fill) + str + std::string(uright, fill);
    return ret;
}

std::string util::rand_string(size_t length)
{
    std::string range = "0123456789QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm";
    std::string ret = "";
    util::rand r;

    for (size_t i = 0; i < length; ++i)
    {
        ret += range[r.get_int<unsigned long>(1, range.length()) - 1];
    }

    return ret;
}

std::string util::prettybytes(std::string str)
{
    std::string ret = "";
    for (size_t i = 0; i < str.length(); ++i)
    {
        ret += std::to_string((uint8_t)str[i]);
        if (i != str.length() - 1)
            ret += "_";
    }

    return ret;
}

std::string util::strtolower(std::string str)
{
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);

    return ret;
}

std::string util::strtoupper(std::string str)
{
    std::string ret = str;
    std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);

    return ret;
}

bool util::isalpha(std::string str)
{
    for (char c : str)
    {
        if (!std::isalpha(c))
        {
            return false;
        }
    }

    return true;
}

bool util::isnumeric(std::string str)
{
    return str.find_first_not_of("\t 0123456789") == std::string::npos;
}

bool util::isplaintext(std::string str)
{
    for (char c : str)
    {
        if (c < 32 || c > 126)
        {
            return false;
        }
    }

    return true;
}

std::string util::sha256(std::string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ret;
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        ret << std::hex << std::setw(2) << std::setfill('0') << (unsigned short)hash[i];
    }

    return ret.str();
}

util::rand::rand()
{
    this->engine = std::mt19937(get_eo_timestamp());
}

long int util::get_tnl(uint8_t level, size_t exp)
{
   return (long int)(std::floor(std::pow(level + 1, 3.0) * 133.1) - exp);
}
