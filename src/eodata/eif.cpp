/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "eif.hpp"

eodata::EIF::EIF()
{

}

eodata::EIF::EIF(const std::string &filename)
{
    operator=(filename);
}

void eodata::EIF::operator=(const std::string &filename)
{
    this->filename = filename;
    this->raw_file = eodata::get_file(filename);

    out::info("Loaded %u items from '%s'.", this->parse(), out::colorize(this->filename, out::COLOR_WHITE).c_str());
}

size_t eodata::EIF::parse()
{
    PacketReader eif(0);
    eif.Load(this->raw_file);
    std::string header = eif.get_string(3);
    this->checksum = eif.get_string(6);

    if (header != "EIF")
    {
        exit(eodata::raise_data_error(this->filename, "Wrong header \"" + header + " (expected \"EIF\")"));
    }

    if (eif.get_char() != 1)
    {
        exit(eodata::raise_data_error(this->filename, "This is not a valid EIF file."));
    }

    size_t current_id;
    for (current_id = 1; !eif.is_end(); ++current_id)
    {
        item_t newitem;
        newitem.name = eif.get_string(eif.get_int(1));
        if (!util::isplaintext(newitem.name))
        {
            exit(eodata::raise_data_error(this->filename, "The file contains malformed data."));
        }

        if (newitem.name == "eof")
        {
            --current_id;
            break;
        }

        newitem.gfx = eif.get_int(2);
        newitem.type = (item_type)eif.get_int(1);
        newitem.subtype = (item_subtype)eif.get_int(1);
        newitem.rarity = (item_rarity)eif.get_int(1);
        newitem.hp = eif.get_int(2);
        newitem.tp = eif.get_int(2);
        newitem.damage.min = eif.get_int(2);
        newitem.damage.max = eif.get_int(2);
        newitem.accuracy = eif.get_int(2);
        newitem.evasion = eif.get_int(2);
        newitem.defense = eif.get_int(2);
        eif.get_int(1); // unknown value
        newitem.stats.str = eif.get_int(1);
        newitem.stats.intelligence = eif.get_int(1);
        newitem.stats.wis = eif.get_int(1);
        newitem.stats.agi = eif.get_int(1);
        newitem.stats.con = eif.get_int(1);
        newitem.stats.cha = eif.get_int(1);
        newitem.resistance.light = eif.get_int(1);
        newitem.resistance.dark = eif.get_int(1);
        newitem.resistance.earth = eif.get_int(1);
        newitem.resistance.air = eif.get_int(1);
        newitem.resistance.water = eif.get_int(1);
        newitem.resistance.fire = eif.get_int(1);
        uint32_t spec1 = eif.get_int(3);
        uint8_t spec2 = eif.get_int(1);
        /* These 2 have more than 1 function. */
        newitem.warp.map = spec1;
        newitem.warp.x = spec2;
        newitem.warp.y = eif.get_int(1);
        newitem.look = spec1;
        if (spec2 < 2)
        {
            newitem.gender = spec2;
        }
        newitem.level_requirement = eif.get_int(2);
        newitem.class_requirement = eif.get_int(2);
        newitem.stats_requirements.str = eif.get_int(2);
        newitem.stats_requirements.intelligence = eif.get_int(2);
        newitem.stats_requirements.wis = eif.get_int(2);
        newitem.stats_requirements.agi = eif.get_int(2);
        newitem.stats_requirements.con = eif.get_int(2);
        newitem.stats_requirements.cha = eif.get_int(2);
        eif.get_int(1); // unknown value
        eif.get_int(1); // unknown value
        newitem.weight = eif.get_int(1);
        eif.get_int(1); // unknown value
        newitem.size = (item_size)eif.get_int(1);

        this->items[current_id] = newitem;
    }

    return current_id;
}

eodata::EIF::item_t eodata::EIF::get(size_t id)
{
    item_t ret;
    if (this->items.count(id) != 0)
    {
        ret = this->items[id];
    }

    return ret;
}
