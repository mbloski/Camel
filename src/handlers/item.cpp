/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::junk(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);
    size_t amount = packet->get_int(3);

    if (client->ingame && client->character->inventory->get_amount(id) >= amount)
    {
        client->character->inventory->remove(id, amount);
        client->session_vals["weight"] = client->session_vals["weight"] - server->pub.items.get(id).weight * amount;

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_JUNK);
        client->get_packet_builder()->add_int(id, 2);
        client->get_packet_builder()->add_int(amount, 3);
        client->get_packet_builder()->add_int(client->character->inventory->get_amount(id), 4);
        client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
        client->get_packet_builder()->add_int(70, 1);
        client->Send();
    }
}

void handler::drop(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);
    size_t amount = packet->get_int(3);
    uint8_t x = packet->get_int(1);
    uint8_t y = packet->get_int(1);

    if (x == EO_INT2 + 1 && y == EO_INT2 + 1)
    {
        x = client->character->position.x;
        y = client->character->position.y;
    }

    if (client->character->inventory->get_amount(id) < amount
            || amount == 0
            || server->pub.items.get(id).rarity == eodata::EIF::RARITY_LORE
            || !client->character->in_range_of(client->character->position.map, x, y, 2)
            || !server->maps->get(client->character->position.map).is_walkable(x, y))
    {
        return;
    }

    eodata::EMF::drop_t newdrop;
    newdrop.id = id;
    newdrop.amount = amount;
    newdrop.pos = std::make_pair(x, y);
    newdrop.effect = 0;
    newdrop.pid = client->id;

    client->character->inventory->remove(id, amount);
    client->session_vals["weight"] -= server->pub.items.get(id).weight * amount;

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_DROP);
    client->get_packet_builder()->add_int(newdrop.id, 2);
    client->get_packet_builder()->add_int(newdrop.amount, 3);
    client->get_packet_builder()->add_int(client->character->inventory->get_amount(id), 4);
    client->get_packet_builder()->add_int(server->maps->get(client->character->position.map).drops.size(), 2);
    client->get_packet_builder()->add_int(x, 1);
    client->get_packet_builder()->add_int(y, 1);
    client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
    client->get_packet_builder()->add_int(70, 1); // max weight
    client->Send();

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_ADD);
            c->get_packet_builder()->add_int(newdrop.id, 2);
            c->get_packet_builder()->add_int(server->maps->get(client->character->position.map).drops.size(), 2);
            c->get_packet_builder()->add_int(newdrop.amount, 3);
            c->get_packet_builder()->add_int(x, 1);
            c->get_packet_builder()->add_int(y, 1);
            c->Send();
        }
    }

    server->maps->add_item(client->character->position.map, newdrop);
}

void handler::pick(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t id = packet->get_int(2);

    bool authorized = false;
    bool drop_protected = false;
    size_t drop_id = 0;
    eodata::EMF::drop_t drop;

    for (std::pair<size_t, eodata::EMF::drop_t> i : server->maps->get(client->character->position.map).drops)
    {
        drop_id = i.first;
        drop = i.second;

        if (drop_id != id || drop.archived)
        {
            continue;
        }

        if (client->character->in_range_of(client->character->position.map, drop.pos.first, drop.pos.second, 2))
        {
            if (client->id != drop.pid && std::time(0) - drop.time < 6)
            {
                drop_protected = true;
            }
            else
            {
                authorized = true;
            }
            break;
        }
    }

    if (drop_protected)
    {
        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_SPEC);
        client->get_packet_builder()->add_int(2, 2);
        client->Send();
    }

    if (authorized)
    {
        server->maps->archive_item(client->character->position.map, id);
        server->AddItem(client, drop.id, drop.amount, drop_id);

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range_of(client->character->position.map, drop.pos.first, drop.pos.second) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_REMOVE);
                c->get_packet_builder()->add_int(drop_id, 2);
                c->Send();
            }
        }
    }
}
