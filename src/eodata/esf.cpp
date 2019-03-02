/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "esf.hpp"

eodata::ESF::ESF()
{

}

eodata::ESF::ESF(const std::string &filename)
{
    operator=(filename);
}

void eodata::ESF::operator=(const std::string &filename)
{
    this->filename = filename;
    this->raw_file = eodata::get_file(filename);

    out::info("Loaded %u spells from '%s'.", this->parse(), out::colorize(this->filename, out::COLOR_WHITE).c_str());
}

size_t eodata::ESF::parse()
{
    PacketReader esf(0);
    esf.Load(this->raw_file);
    std::string header = esf.get_string(3);
    this->checksum = esf.get_string(6);

    if (header != "ESF")
    {
        exit(eodata::raise_data_error(this->filename, "Wrong header \"" + header + " (expected \"ESF\")"));
    }

    if (esf.get_char() != 1)
    {
        exit(eodata::raise_data_error(this->filename, "This is not a valid ESF file."));
    }

    size_t current_id;
    for (current_id = 1; !esf.is_end(); ++current_id)
    {
        spell_t newspell;
        size_t name_length = esf.get_int(1);
        size_t invoc_length = esf.get_int(1);
        newspell.name = esf.get_string(name_length);
        newspell.invocation = esf.get_string(invoc_length);
        if (!util::isplaintext(newspell.name) || !util::isplaintext(newspell.invocation))
        {
            exit(eodata::raise_data_error(this->filename, "The file contains malformed data."));
        }

        if (newspell.name == "eof")
        {
            --current_id;
            break;
        }

        newspell.icon = esf.get_int(2);
        newspell.gfx = esf.get_int(2);
        newspell.tp = esf.get_int(2);
        newspell.sp = esf.get_int(2);
        newspell.casttime = esf.get_int(1);
        esf.get_int(1); // useless
        esf.get_int(1); // useless
        newspell.type = (spell_type)esf.get_int(1);
        esf.get_string(5); // skip useless data
        newspell.purpose = (spell_purpose)esf.get_int(1);
        newspell.target = (spell_target)esf.get_int(1);
        esf.get_string(4); // skip useless data
        newspell.damage.min = esf.get_int(2);
        newspell.damage.max = esf.get_int(2);
        newspell.accuracy = esf.get_int(2);
        esf.get_string(5); // skip useless data
        newspell.hp = esf.get_int(2);
        esf.get_string(15); // skip A LOT OF useless data
        /* Not sure how much of the useless data is really useless. TODO: check it. */

        this->spells[current_id] = newspell;
    }

    return current_id;
}

eodata::ESF::spell_t eodata::ESF::get(size_t id)
{
    spell_t ret;
    if (this->spells.count(id) != 0)
    {
        ret = this->spells[id];
    }

    return ret;
}
