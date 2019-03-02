/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "character.hpp"

Inventory::Inventory(std::string str)
{
    for (std::string item : util::tokenize(str, '|'))
    {
        if (item.empty())
        {
            continue;
        }
        std::vector<std::string> item_v = util::tokenize(item, ',');
        size_t item_id = stoul(item_v.at(0));
        size_t amount = stoul(item_v.at(1));
        this->inventory[item_id] = amount;
    }
}

void Inventory::add(size_t id, size_t amount)
{
    size_t current_amount = this->inventory[id];
    this->inventory[id] = current_amount + amount;
}

void Inventory::remove(size_t id, size_t amount)
{
    size_t current_amount = this->inventory[id];
    size_t newamt = amount > current_amount ? 0 : current_amount - amount;
    this->inventory[id] = newamt;
}

std::string Inventory::get_serialized()
{
    std::string ret = "";
    for (std::pair<size_t, size_t> i : this->inventory)
    {
        if (i.first != 1 && i.second == 0)
        {
            continue;
        }
        ret += std::to_string(i.first) + "," + std::to_string(i.second) + "|";
    }

    if (ret.length() > 0)
        ret.pop_back();

    return ret;
}

size_t Inventory::get_amount(size_t id)
{
    return this->inventory[id];
}

Paperdoll::Paperdoll(std::string str)
{
    std::vector<std::string> tok = util::tokenize(str, ',');
    if (tok.size() < 15)
    {
        return;
    }

    this->armor = (uint16_t)std::stoi(tok[0]);
    this->hat = (uint16_t)std::stoi(tok[1]);
    this->boots = (uint16_t)std::stoi(tok[2]);
    this->weapon = (uint16_t)std::stoi(tok[3]);
    this->shield = (uint16_t)std::stoi(tok[4]);
    this->gloves = (uint16_t)std::stoi(tok[5]);
    this->belt = (uint16_t)std::stoi(tok[6]);
    this->necklace = (uint16_t)std::stoi(tok[7]);
    this->rings.first = (uint16_t)std::stoi(tok[8]);
    this->rings.second = (uint16_t)std::stoi(tok[9]);
    this->bracelets.first = (uint16_t)std::stoi(tok[10]);
    this->bracelets.second = (uint16_t)std::stoi(tok[11]);
    this->bracers.first = (uint16_t)std::stoi(tok[12]);
    this->bracers.second = (uint16_t)std::stoi(tok[13]);
    this->gem = (uint8_t)std::stoi(tok[14]);
}

std::string Paperdoll::get_serialized()
{
    std::string ret = "";
    ret += std::to_string(this->armor) + ",";
    ret += std::to_string(this->hat) + ",";
    ret += std::to_string(this->boots) + ",";
    ret += std::to_string(this->weapon) + ",";
    ret += std::to_string(this->shield) + ",";
    ret += std::to_string(this->gloves) + ",";
    ret += std::to_string(this->belt) + ",";
    ret += std::to_string(this->necklace) + ",";
    ret += std::to_string(this->rings.first) + ",";
    ret += std::to_string(this->rings.second) + ",";
    ret += std::to_string(this->bracelets.first) + ",";
    ret += std::to_string(this->bracelets.second) + ",";
    ret += std::to_string(this->bracers.first) + ",";
    ret += std::to_string(this->bracers.second) + ",";
    ret += std::to_string(this->gem);

    return ret;
}

Character::Character(std::shared_ptr<sql::ResultSet> sqldata)
{
    this->id = sqldata->getUInt("id");
    this->name = sqldata->getString("name");
    this->gender = (uint8_t)sqldata->getUInt("sex");
    this->race = (uint8_t)sqldata->getUInt("race");
    this->hairstyle = (uint8_t)sqldata->getUInt("hairstyle");
    this->haircolor = (uint8_t)sqldata->getUInt("haircolor");
    this->home = sqldata->getString("home");
    this->guild = sqldata->getString("guild");
    this->guild_rank = (uint8_t)sqldata->getUInt("guildrank");
    this->guild_rank_str = sqldata->getString("guildrank_str");
    this->title = sqldata->getString("title");
    this->exp = sqldata->getUInt("exp");
    this->level = (uint8_t)sqldata->getUInt("level");
    this->admin = (uint8_t)sqldata->getUInt("admin");
    this->position.map = (uint16_t)sqldata->getUInt("map");
    this->position.x = (uint8_t)sqldata->getUInt("x");
    this->position.y = (uint8_t)sqldata->getUInt("y");
    this->position.direction = (uint8_t)sqldata->getUInt("direction");
    this->hp = sqldata->getUInt("hp");
    this->tp = sqldata->getUInt("tp");
    this->class_id = sqldata->getUInt("class");
    this->stats.str = (uint16_t)sqldata->getUInt("str");
    this->stats.intelligence = (uint16_t)sqldata->getUInt("int");
    this->stats.wis = (uint16_t)sqldata->getUInt("wis");
    this->stats.agi = (uint16_t)sqldata->getUInt("agi");
    this->stats.con = (uint16_t)sqldata->getUInt("con");
    this->stats.cha = (uint16_t)sqldata->getUInt("cha");
    this->karma = sqldata->getUInt("karma");
    this->usage = sqldata->getUInt("usage");
    this->arena_wins = sqldata->getUInt("arena_wins");
    this->arena_kills = sqldata->getUInt("arena_kills");
    this->arena_deaths = sqldata->getUInt("arena_deaths");
    this->sitting = (uint8_t)sqldata->getUInt("sitting");
    this->hidden = sqldata->getBoolean("hidden");
    this->inventory = new Inventory(sqldata->getString("inventory"));
    this->paperdoll = new Paperdoll(sqldata->getString("paperdoll"));
}

void Character::UpdateStats()
{
    while (util::get_tnl(this->level, this->exp) <= 0)
    {
        this->level += 1;
    }
}

bool Character::in_range(Character *character, size_t distance)
{
    if (this->position.map == character->position.map && std::abs(this->position.x - character->position.x) + std::abs(this->position.y - character->position.y) <= int(distance))
    {
        return true;
    }

    return false;
}

bool Character::in_range_of(size_t map, uint8_t x, uint8_t y, size_t distance)
{
    if (this->position.map == map && std::abs(this->position.x - x) + std::abs(this->position.y - y) <= int(distance))
    {
        return true;
    }

    return false;
}

Character::~Character()
{
    delete this->inventory;
    delete this->paperdoll;
    out::info("Character %s destroyed.", this->name.c_str());
}
