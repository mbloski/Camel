/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "emf.hpp"

eodata::EMF::EMF(size_t howmany, const std::string &directory)
{
    this->mapcount = howmany;
    operator=(directory);
}

void eodata::EMF::operator=(const std::string &directory)
{
    std::string dir = directory;
    if (directory.back() != '/')
    {
        dir += "/";
    }
    this->directory = dir;

    out::info("Loading maps from '%s'...", out::colorize(dir, out::COLOR_WHITE).c_str());

    size_t i;
    for (i = 1; i <= this->mapcount; ++i)
    {
        this->parse(i);
    }

    out::info("Loaded %u maps.", this->maps.size());
}

size_t eodata::EMF::parse(size_t id)
{
    std::stringstream mapid;
    mapid << std::setw(5) << std::setfill('0') << id;
    std::string fullpath = this->directory + mapid.str() + ".emf";
#ifdef DEBUG
    out::debug("Loading map '%s'...", out::colorize(fullpath, out::COLOR_WHITE).c_str());
#endif // DEBUG
    map_t newmap;
    newmap.path = fullpath;

    PacketReader emf(0);
    std::string file = eodata::get_file(newmap.path);
    emf.Load(file);
    std::string header = emf.get_string(3);
    std::vector<uint8_t> filesize_eoint = eoencoding::eoint_encode(file.length(), 3);

    newmap.checksum = emf.get_string(4) + std::string(filesize_eoint.begin(), filesize_eoint.end());

    if (header != "EMF")
    {
        exit(eodata::raise_data_error(this->maps[id].path, "Wrong header \"" + header + " (expected \"EMF\")"));
    }

    std::string namebuf = emf.get_string(24);
    std::reverse(namebuf.begin(), namebuf.end());

    newmap.id = id;

    bool flippy = false;
    for (char &c : namebuf)
    {
        if (flippy)
        {
            if (c >= 34 && c <= 79)
            {
                c = 113 - c;
            }
            else if (c >= 80 && c <= 126)
            {
                c = 205 - c;
            }
        }
        else
        {
            if (c >= 34 && c <= 126)
            {
                c = 159 - c;
            }
        }

        flippy = !flippy;
    }

    newmap.name = namebuf.substr(0, namebuf.find(eoencoding::BYTE_COMMA));
    newmap.pk = (bool)emf.get_int(1);

    emf.get_string(5); // 4 unknown attributes (ABCDD)

    newmap.width = emf.get_int(1);
    newmap.height = emf.get_int(1);

    emf.get_string(3); // 2 unknown attributes (AAB)

    newmap.scrollable = (bool)emf.get_int(1);
    newmap.relogx = emf.get_int(1);
    newmap.relogy = emf.get_int(1);

    emf.get_string(1); // 1 unknown attribute

    uint8_t outersize;

    outersize = emf.get_int(1);
    for (size_t i = 0; i < outersize; ++i)
    {
        map_t::npc newnpc;
        uint8_t xloc = emf.get_int(1);
        uint8_t yloc = emf.get_int(1);
        newnpc.id = emf.get_int(2);
        newnpc.type = emf.get_int(1);
        newnpc.time = emf.get_int(2);
        newnpc.amount = emf.get_int(1);

        newmap.tiles[std::make_pair(xloc, yloc)].npc = newnpc;
    }

    outersize = emf.get_int(1);
    for (size_t i = 0; i < outersize; ++i)
    {
        emf.get_int(1); // xloc
        emf.get_int(1); // yloc
        emf.get_int(2); // unknown
    }

    outersize = emf.get_int(1);
    for (size_t i = 0; i < outersize; ++i)
    {
        map_t::chest_item newitem;
        uint8_t xloc = emf.get_int(1);
        uint8_t yloc = emf.get_int(1);
        uint8_t key = emf.get_int(2);
        newitem.slot = emf.get_int(1);
        newitem.id = emf.get_int(2);
        newitem.time = emf.get_int(2);
        newitem.amount = emf.get_int(3);

        /* It's not so good the key is being rewritten every iteration.
           This is the consequence of having the key ID stored in the same place as items, though. */
        newmap.tiles[std::make_pair(xloc, yloc)].chest.key = key;
        newmap.tiles[std::make_pair(xloc, yloc)].chest.items.push_back(newitem);
    }

    outersize = emf.get_int(1);
    for (size_t i = 0; i < outersize; ++i)
    {
        uint8_t yloc = emf.get_int(1);
        uint8_t innersize = emf.get_int(1);
        for (size_t j = 0; j < innersize; ++j)
        {
            uint8_t xloc = emf.get_int(1);
            tile_t attrib = (tile_t)emf.get_int(1);

            newmap.tiles[std::make_pair(xloc, yloc)].type = attrib;
        }
    }

    outersize = emf.get_int(1);

    for (size_t i = 0; i < outersize; ++i)
    {
        uint8_t yloc = emf.get_int(1);
        uint8_t innersize = emf.get_int(1);
        for (size_t j = 0; j < innersize; ++j)
        {
            map_t::warp newwarp;
            uint8_t xloc = emf.get_int(1);
            newwarp.map = emf.get_int(2);
            newwarp.x = emf.get_int(1);
            newwarp.y = emf.get_int(1);
            newwarp.level = emf.get_int(1);
            newwarp.type = (warp_t)emf.get_int(2);

            if (newwarp.type == WARP_NORMAL)
            {
                newwarp.open = true;
            }
            else
            {
                newwarp.open = false;
            }

            newmap.tiles[std::make_pair(xloc, yloc)].warp = newwarp;
        }
    }

    this->maps[id] = newmap;
    return this->maps[id].tiles.size();
}

bool eodata::EMF::add_item(size_t id, eodata::EMF::drop_t drop)
{
    if (this->maps.count(id) != 0)
    {
        this->maps[id].drops.push_back(std::make_pair(this->maps[id].drops.size(), drop));
        return true;
    }

    return false;
}

bool eodata::EMF::archive_item(size_t id, size_t drop_id)
{
    if (this->maps.count(id) != 0)
    {
        if (this->maps[id].drops.size() < drop_id)
        {
            return false;
        }

        (this->maps[id].drops.begin() + drop_id)->second.archived = true;
        return true;
    }

    return false;
}

bool eodata::EMF::update(size_t id, eodata::EMF::map_t newmap)
{
    if (this->maps.count(id) != 0)
    {
        this->maps[id] = newmap;
        return true;
    }

    return false;
}

eodata::EMF::map_t eodata::EMF::get(size_t id)
{
    map_t ret;
    if (this->maps.count(id) != 0)
    {
        ret = this->maps[id];
    }

    return ret;
}

bool eodata::EMF::map_t::is_walkable(uint8_t x, uint8_t y)
{
    tile_t type = this->tiles[std::make_pair(x, y)].type;

    if (type != TILE_WALL
    && type != TILE_CHAIR_ALL
    && type != TILE_CHAIR_DOWN
    && type != TILE_CHAIR_DOWN_RIGHT
    && type != TILE_CHAIR_LEFT
    && type != TILE_CHAIR_RIGHT
    && type != TILE_CHAIR_UP
    && type != TILE_CHAIR_UP_LEFT
    && type != TILE_CHEST
    && type != TILE_BANK
    && type != TILE_EDGE
    && type != TILE_BOARD1
    && type != TILE_BOARD2
    && type != TILE_BOARD3
    && type != TILE_BOARD4
    && type != TILE_BOARD5
    && type != TILE_BOARD6
    && type != TILE_BOARD7
    && type != TILE_BOARD8
    && type != TILE_JUKEBOX)
    {
        return true;
    }

    return false;
}

