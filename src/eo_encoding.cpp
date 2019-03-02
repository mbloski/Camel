/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "eo_encoding.hpp"

std::string eoencoding::string_interleave(std::string str)
{
    std::string ret(str);
    int i = 0, j = 0;

    for (; i < int(str.length()); i += 2)
    {
        ret[i] = str[j++];
    }

    --i;

    if (str.length() % 2 != 0)
    {
        i -= 2;
    }

    for (; i >= 0; i -= 2)
    {
        ret[i] = str[j++];
    }

    return ret;
}

std::string eoencoding::string_deinterleave(std::string str)
{
    std::string ret(str);
    int i = 0, j = 0;

    for (; i < int(str.length()); i += 2)
    {
        ret[j++] = str[i];
    }

    --i;

    if (str.length() % 2 != 0)
    {
        i -= 2;
    }

    for (; i >= 0; i -= 2)
    {
        ret[j++] = str[i];
    }

    return ret;
}

std::string eoencoding::xor128(std::string str)
{
    for (char &c : str)
    {
        c ^= 0x80;
        if (c == 0)
        {
            c = 0x80;
        }
    }

    return str;
}

std::string eoencoding::dwind(std::string str, uint8_t multi)
{
    std::string ret(str);
    int seqlen = 0;

    if (multi == 0)
    {
        return str;
    }

    size_t i;
    for (i = 0; i < ret.length(); ++i)
    {
        if (i != ret.length() && ((unsigned char)ret[i] % multi) == 0)
        {
            ++seqlen;
        }
        else
        {
            if (seqlen > 1)
            {
                std::reverse(ret.begin() + i - seqlen, ret.begin() + i);
            }

            seqlen = 0;
        }
    }

    /* Any remaining bytes to swap? */
    if (seqlen > 1)
    {
        std::reverse(ret.begin() + i - seqlen, ret.begin() + i);
    }

    return ret;
}

std::vector<uint8_t> eoencoding::eoint_encode(unsigned int n, uint8_t len)
{
    std::vector<uint8_t> ret = {0, 0, 0, 0};

    uint8_t real_len = 1;
    unsigned int n_copy = n;

    if (n_copy >= EO_INT4)
    {
        ret[3] = static_cast<uint8_t>(n / double(EO_INT4) + 1);
        n = n % EO_INT4;
    }

    if (n_copy >= EO_INT3)
    {
        ret[2] = static_cast<uint8_t>(n / double(EO_INT3) + 1);
        n = n % EO_INT3;
    }

    if (n_copy >= EO_INT2)
    {
        ret[1] = static_cast<uint8_t>(n / double(EO_INT2) + 1);
        n = n % EO_INT2;
    }

    if (n_copy >= EO_INT2) ++real_len;
    if (n_copy >= EO_INT3) ++real_len;
    if (n_copy >= EO_INT4) ++real_len;

    ret[0] = static_cast<uint8_t>(n + 1);
    ret.resize(real_len);

    if (len == 0)
    {
        return ret;
    }

    if (len >= real_len)
    {
        ret.insert(ret.end(), len - real_len, eoencoding::get_eobyte(BYTE_NULL));
    }
    else
    {
        out::warning("Please note: eoint_encode()'s length is higher than desired.");
        out::warning("^ the value will be truncated!");
        ret.resize(len);
    }

    return ret;
}

unsigned int eoencoding::eoint_decode(std::vector<uint8_t> eoint)
{
    if (eoint.size() < 4)
    {
        eoint.insert(eoint.end(), 4 - eoint.size(), eoencoding::get_eobyte(BYTE_NULL));
    }

    for (uint8_t &b : eoint)
    {
        if (b == 0 || b == 254)
        {
            b = 1;
        }
    }

    for (uint8_t &b : eoint)
    {
        --b;
    }

    return (eoint[3] * EO_INT4 + eoint[2] * EO_INT3 + eoint[1] * EO_INT2 + eoint[0]);
}

unsigned char eoencoding::get_eobyte(eoencoding::EOByte byte)
{
    switch(byte)
    {
        case BYTE_NULL:
            return 254;
        break;
        case BYTE_COMMA:
            return 255;
        break;
    }

    /* To silence the compiler. */
    return 0;
}
