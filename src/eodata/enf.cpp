/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "enf.hpp"

eodata::ENF::ENF()
{

}

eodata::ENF::ENF(const std::string &filename)
{
    operator=(filename);
}

void eodata::ENF::operator=(const std::string &filename)
{
    this->filename = filename;
    this->raw_file = eodata::get_file(filename);

    out::info("Loaded %u NPCs from '%s'.", this->parse(), out::colorize(this->filename, out::COLOR_WHITE).c_str());
}

size_t eodata::ENF::parse()
{
    PacketReader enf(0);
    enf.Load(this->raw_file);
    std::string header = enf.get_string(3);
    this->checksum = enf.get_string(6);

    if (header != "ENF")
    {
        exit(eodata::raise_data_error(this->filename, "Wrong header \"" + header + " (expected \"ENF\")"));
    }

    if (enf.get_char() != 1)
    {
        exit(eodata::raise_data_error(this->filename, "This is not a valid ENF file."));
    }

    size_t current_id;
    for (current_id = 1; !enf.is_end(); ++current_id)
    {
        npc_t newnpc;
        newnpc.name = enf.get_string(enf.get_int(1));
        if (!util::isplaintext(newnpc.name))
        {
            exit(eodata::raise_data_error(this->filename, "The file contains malformed data."));
        }

        if (newnpc.name == "eof")
        {
            --current_id;
            break;
        }

        newnpc.gfx = enf.get_int(2);
        enf.get_int(1);
        newnpc.boss = enf.get_int(2);
        newnpc.child = enf.get_int(2);
        newnpc.type = (npc_type)enf.get_int(2);
        newnpc.id = enf.get_int(2);
        newnpc.hp = enf.get_int(3);
        enf.get_string(2); // unknown. Always 0
        newnpc.damage.min = enf.get_int(2);
        newnpc.damage.max = enf.get_int(2);
        newnpc.accuracy = enf.get_int(2);
        newnpc.evasion = enf.get_int(2);
        newnpc.defense = enf.get_int(2);
        enf.get_string(10); // Some values I'll most likely never use.
        newnpc.exp = enf.get_int(3);

        this->npcs[current_id] = newnpc;
    }

    return current_id;
}

eodata::ENF::npc_t eodata::ENF::get(size_t id)
{
    npc_t ret;
    if (this->npcs.count(id) != 0)
    {
        ret = this->npcs[id];
    }

    return ret;
}
