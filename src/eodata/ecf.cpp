/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "ecf.hpp"

eodata::ECF::ECF()
{

}

eodata::ECF::ECF(const std::string &filename)
{
    operator=(filename);
}

void eodata::ECF::operator=(const std::string &filename)
{
    this->filename = filename;
    this->raw_file = eodata::get_file(filename);

    out::info("Loaded %u classes from '%s'.", this->parse(), out::colorize(this->filename, out::COLOR_WHITE).c_str());
}

size_t eodata::ECF::parse()
{
    PacketReader ecf(0);
    ecf.Load(this->raw_file);
    std::string header = ecf.get_string(3);
    this->checksum = ecf.get_string(6);

    if (header != "ECF")
    {
        exit(eodata::raise_data_error(this->filename, "Wrong header \"" + header + " (expected \"ECF\")"));
    }

    if (ecf.get_char() != 1)
    {
        exit(eodata::raise_data_error(this->filename, "This is not a valid ECF file."));
    }

    size_t current_id;
    for (current_id = 1; !ecf.is_end(); ++current_id)
    {
        class_t newclass;
        newclass.name = ecf.get_string(ecf.get_int(1));
        if (!util::isplaintext(newclass.name))
        {
            exit(eodata::raise_data_error(this->filename, "The file contains malformed data."));
        }

        if (newclass.name == "eof")
        {
            --current_id;
            break;
        }

        newclass.id = current_id;
        newclass.base = ecf.get_int(1);
        newclass.type = ecf.get_int(1);
        newclass.stats.str = ecf.get_int(2);
        newclass.stats.intelligence = ecf.get_int(2);
        newclass.stats.wis = ecf.get_int(2);
        newclass.stats.agi = ecf.get_int(2);
        newclass.stats.con = ecf.get_int(2);
        newclass.stats.cha = ecf.get_int(2);
        this->classes[current_id] = newclass;
    }

    return current_id;
}

eodata::ECF::class_t eodata::ECF::get(size_t id)
{
    class_t ret;
    if (this->classes.count(id) != 0)
    {
        ret = this->classes[id];
    }

    return ret;
}
