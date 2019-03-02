/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::open_paperdoll(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->id == id)
        {
            client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_REPLY);
            client->get_packet_builder()->add_string(c->character->name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->home);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->partner);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->title);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->guild_name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(c->character->guild_rank_str);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_int(id, 2);
            client->get_packet_builder()->add_int(c->character->class_id, 1);
            client->get_packet_builder()->add_int(c->character->gender, 1);
            client->get_packet_builder()->add_int(0, 1);
            client->get_packet_builder()->add_int(c->character->paperdoll->boots, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->gem, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->gloves, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->belt, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->armor, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->necklace, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->hat, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->shield, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->weapon, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->rings.first, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->rings.second, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->bracelets.first, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->bracelets.second, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->bracers.first, 2);
            client->get_packet_builder()->add_int(c->character->paperdoll->bracers.second, 2);
            client->get_packet_builder()->add_int(c->character->admin, 1); // TODO: party icons
            client->Send();
            return;
        }
    }
}

void handler::equip(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);
    uint8_t slot = packet->get_int(1);

    if (server->pub.items.get(id).type != eodata::EIF::TYPE_WEAPON
            && server->pub.items.get(id).type != eodata::EIF::TYPE_SHIELD
            && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMOR
            && server->pub.items.get(id).type != eodata::EIF::TYPE_HAT
            && server->pub.items.get(id).type != eodata::EIF::TYPE_SHOES
            && server->pub.items.get(id).type != eodata::EIF::TYPE_GLOVES
            && server->pub.items.get(id).type != eodata::EIF::TYPE_MISC
            && server->pub.items.get(id).type != eodata::EIF::TYPE_BELT
            && server->pub.items.get(id).type != eodata::EIF::TYPE_NECKLACE
            && server->pub.items.get(id).type != eodata::EIF::TYPE_RING
            && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMLET
            && server->pub.items.get(id).type != eodata::EIF::TYPE_BRACER)
    {
        /* Not wearable */
#ifdef DEBUG
        out::debug("%s wanted to equip unwearable item %d", client->character->name.c_str(), id);
#endif // DEBUG
        return;
    }

    if (server->pub.items.get(id).type == eodata::EIF::TYPE_ARMOR
            && server->pub.items.get(id).gender != client->character->gender)
    {
#ifdef DEBUG
        out::debug("%s wanted to equip wrong gender item %d", client->character->name.c_str(), id);
#endif // DEBUG
        return;
    }

    if (client->ingame && client->character->inventory->get_amount(id) > 0)
    {
        client->character->inventory->remove(id);

        /* ugh. */
        switch (server->pub.items.get(id).type)
        {
        case eodata::EIF::TYPE_WEAPON:
            if (client->character->paperdoll->weapon != 0) return;
            client->character->paperdoll->weapon = id;
            break;
        case eodata::EIF::TYPE_SHIELD:
            if (client->character->paperdoll->shield != 0) return;
            client->character->paperdoll->shield = id;
            break;
        case eodata::EIF::TYPE_ARMOR:
            if (client->character->paperdoll->armor != 0) return;
            client->character->paperdoll->armor = id;
            break;
        case eodata::EIF::TYPE_HAT:
            if (client->character->paperdoll->hat != 0) return;
            client->character->paperdoll->hat = id;
            break;
        case eodata::EIF::TYPE_SHOES:
            if (client->character->paperdoll->boots != 0) return;
            client->character->paperdoll->boots = id;
            break;
        case eodata::EIF::TYPE_GLOVES:
            if (client->character->paperdoll->gloves != 0) return;
            client->character->paperdoll->gloves = id;
            break;
        case eodata::EIF::TYPE_MISC:
            if (client->character->paperdoll->gem != 0) return;
            client->character->paperdoll->gem = id;
            break;
        case eodata::EIF::TYPE_BELT:
            if (client->character->paperdoll->belt != 0) return;
            client->character->paperdoll->belt = id;
            break;
        case eodata::EIF::TYPE_NECKLACE:
            if (client->character->paperdoll->necklace != 0) return;
            client->character->paperdoll->necklace = id;
            break;
        case eodata::EIF::TYPE_RING:
            if (slot == 0)
            {
                if (client->character->paperdoll->rings.first != 0) return;
                client->character->paperdoll->rings.first = id;
            }
            else
            {
                if (client->character->paperdoll->rings.second != 0) return;
                client->character->paperdoll->rings.second = id;
            }
            break;
        case eodata::EIF::TYPE_ARMLET:
            if (slot == 0)
            {
                if (client->character->paperdoll->bracelets.first != 0) return;
                client->character->paperdoll->bracelets.first = id;
            }
            else
            {
                if (client->character->paperdoll->bracelets.second != 0) return;
                client->character->paperdoll->bracelets.second = id;
            }
            break;
        case eodata::EIF::TYPE_BRACER:
            if (slot == 0)
            {
                if (client->character->paperdoll->bracers.first != 0) return;
                client->character->paperdoll->bracers.first = id;
            }
            else
            {
                if (client->character->paperdoll->bracers.second != 0) return;
                client->character->paperdoll->bracers.second = id;
            }
            break;
        default:
            break;
        }

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_AGREE);
        client->get_packet_builder()->add_int(client->id, 2);
        client->get_packet_builder()->add_int(1, 1); // ?
        client->get_packet_builder()->add_int(0, 1); // ?
        client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->boots).look, 2);
        client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->armor).look, 2);
        client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->hat).look, 2);
        client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->weapon).look, 2);
        client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->shield).look, 2);
        client->get_packet_builder()->add_int(id, 2);
        client->get_packet_builder()->add_int(client->character->inventory->get_amount(id), 3);
        client->get_packet_builder()->add_int(slot > 0? 1 : 0, 1);
        client->get_packet_builder()->add_int(client->character->hp, 2);
        client->get_packet_builder()->add_int(client->character->tp, 2);
        client->get_packet_builder()->add_int(client->character->stats.str, 2);
        client->get_packet_builder()->add_int(client->character->stats.intelligence, 2);
        client->get_packet_builder()->add_int(client->character->stats.wis, 2);
        client->get_packet_builder()->add_int(client->character->stats.agi, 2);
        client->get_packet_builder()->add_int(client->character->stats.con, 2);
        client->get_packet_builder()->add_int(client->character->stats.cha, 2);
        client->get_packet_builder()->add_int(0, 2); // min dmg
        client->get_packet_builder()->add_int(0, 2); // max dmg
        client->get_packet_builder()->add_int(0, 2); // acc
        client->get_packet_builder()->add_int(0, 2); // eva
        client->get_packet_builder()->add_int(0, 2); // armor
        client->Send();

        if (server->pub.items.get(id).type != eodata::EIF::TYPE_SHOES
                && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMOR
                && server->pub.items.get(id).type != eodata::EIF::TYPE_HAT
                && server->pub.items.get(id).type != eodata::EIF::TYPE_WEAPON
                && server->pub.items.get(id).type != eodata::EIF::TYPE_SHIELD)
        {
            /* our item doesn't change our avatar so we don't need to announce it */
            return;
        }

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_AVATAR, eoencoding::PACTION_AGREE);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(1, 1);
                c->get_packet_builder()->add_int(0, 1);
                c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->boots).look, 2);
                c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->armor).look, 2);
                c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->hat).look, 2);
                c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->weapon).look, 2);
                c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->shield).look, 2);
                c->Send();
            }
        }
    }
}

void handler::unequip(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);
    uint8_t slot = packet->get_int(1);

    if (server->pub.items.get(id).type != eodata::EIF::TYPE_WEAPON
            && server->pub.items.get(id).type != eodata::EIF::TYPE_SHIELD
            && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMOR
            && server->pub.items.get(id).type != eodata::EIF::TYPE_HAT
            && server->pub.items.get(id).type != eodata::EIF::TYPE_SHOES
            && server->pub.items.get(id).type != eodata::EIF::TYPE_GLOVES
            && server->pub.items.get(id).type != eodata::EIF::TYPE_MISC
            && server->pub.items.get(id).type != eodata::EIF::TYPE_BELT
            && server->pub.items.get(id).type != eodata::EIF::TYPE_NECKLACE
            && server->pub.items.get(id).type != eodata::EIF::TYPE_RING
            && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMLET
            && server->pub.items.get(id).type != eodata::EIF::TYPE_BRACER)
    {
        /* Not wearable */
#ifdef DEBUG
        out::debug("%s wanted to unequip unwearable item %d", client->character->name.c_str(), id);
#endif // DEBUG
        return;
    }

    if (server->pub.items.get(id).rarity == eodata::EIF::RARITY_CURSED)
    {
#ifdef DEBUG
        out::debug("%s wanted to unequip cursed item %d", client->character->name.c_str(), id);
#endif // DEBUG
        return;
    }

    /* ugh. */
    switch (server->pub.items.get(id).type)
    {
    case eodata::EIF::TYPE_WEAPON:
        if (client->character->paperdoll->weapon == 0) return;
        client->character->paperdoll->weapon = 0;
        break;
    case eodata::EIF::TYPE_SHIELD:
        if (client->character->paperdoll->shield == 0) return;
        client->character->paperdoll->shield = 0;
        break;
    case eodata::EIF::TYPE_ARMOR:
        if (client->character->paperdoll->armor == 0) return;
        client->character->paperdoll->armor = 0;
        break;
    case eodata::EIF::TYPE_HAT:
        if (client->character->paperdoll->hat == 0) return;
        client->character->paperdoll->hat = 0;
        break;
    case eodata::EIF::TYPE_SHOES:
        if (client->character->paperdoll->boots == 0) return;
        client->character->paperdoll->boots = 0;
        break;
    case eodata::EIF::TYPE_GLOVES:
        if (client->character->paperdoll->gloves == 0) return;
        client->character->paperdoll->gloves = 0;
        break;
    case eodata::EIF::TYPE_MISC:
        if (client->character->paperdoll->gem == 0) return;
        client->character->paperdoll->gem = 0;
        break;
    case eodata::EIF::TYPE_BELT:
        if (client->character->paperdoll->belt == 0) return;
        client->character->paperdoll->belt = 0;
        break;
    case eodata::EIF::TYPE_NECKLACE:
        if (client->character->paperdoll->necklace == 0) return;
        client->character->paperdoll->necklace = 0;
        break;
    case eodata::EIF::TYPE_RING:
        if (slot == 0)
        {
            if (client->character->paperdoll->rings.first == 0) return;
            client->character->paperdoll->rings.first = 0;
        }
        else
        {
            if (client->character->paperdoll->rings.second == 0) return;
            client->character->paperdoll->rings.second = 0;
        }
        break;
    case eodata::EIF::TYPE_ARMLET:
        if (slot == 0)
        {
            if (client->character->paperdoll->bracelets.first == 0) return;
            client->character->paperdoll->bracelets.first = 0;
        }
        else
        {
            if (client->character->paperdoll->bracelets.second == 0) return;
            client->character->paperdoll->bracelets.second = 0;
        }
        break;
    case eodata::EIF::TYPE_BRACER:
        if (slot == 0)
        {
            if (client->character->paperdoll->bracers.first == 0) return;
            client->character->paperdoll->bracers.first = 0;
        }
        else
        {
            if (client->character->paperdoll->bracers.second == 0) return;
            client->character->paperdoll->bracers.second = 0;
        }
        break;
    default:
        return;
        break;
    }

    client->character->inventory->add(id);

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_REMOVE);
    client->get_packet_builder()->add_int(client->id, 2);
    client->get_packet_builder()->add_int(1, 1); // ?
    client->get_packet_builder()->add_int(0, 1); // ?
    client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->boots).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->armor).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->hat).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->weapon).look, 2);
    client->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->shield).look, 2);
    client->get_packet_builder()->add_int(id, 2);
    client->get_packet_builder()->add_int(slot > 0? 1 : 0, 1);
    client->get_packet_builder()->add_int(client->character->hp, 2);
    client->get_packet_builder()->add_int(client->character->tp, 2);
    client->get_packet_builder()->add_int(client->character->stats.str, 2);
    client->get_packet_builder()->add_int(client->character->stats.intelligence, 2);
    client->get_packet_builder()->add_int(client->character->stats.wis, 2);
    client->get_packet_builder()->add_int(client->character->stats.agi, 2);
    client->get_packet_builder()->add_int(client->character->stats.con, 2);
    client->get_packet_builder()->add_int(client->character->stats.cha, 2);
    client->get_packet_builder()->add_int(0, 2); // min dmg
    client->get_packet_builder()->add_int(0, 2); // max dmg
    client->get_packet_builder()->add_int(0, 2); // acc
    client->get_packet_builder()->add_int(0, 2); // eva
    client->get_packet_builder()->add_int(0, 2); // armor
    client->Send();

    if (server->pub.items.get(id).type != eodata::EIF::TYPE_SHOES
            && server->pub.items.get(id).type != eodata::EIF::TYPE_ARMOR
            && server->pub.items.get(id).type != eodata::EIF::TYPE_HAT
            && server->pub.items.get(id).type != eodata::EIF::TYPE_WEAPON
            && server->pub.items.get(id).type != eodata::EIF::TYPE_SHIELD)
    {
        /* our item doesn't change our avatar so we don't need to announce it */
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_AVATAR, eoencoding::PACTION_AGREE);
            c->get_packet_builder()->add_int(client->id, 2);
            c->get_packet_builder()->add_int(1, 1);
            c->get_packet_builder()->add_int(0, 1);
            c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->boots).look, 2);
            c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->armor).look, 2);
            c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->hat).look, 2);
            c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->weapon).look, 2);
            c->get_packet_builder()->add_int(server->pub.items.get(client->character->paperdoll->shield).look, 2);
            c->Send();
        }
    }
}
