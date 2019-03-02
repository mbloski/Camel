/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::shop_buy(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t item_id = packet->get_int(2);
    size_t amount = packet->get_int(4);
    size_t shop_id = packet->get_int(4);
    bool authed = false;
    eodata::EIF::shop_item_t item;
    size_t buy_price;

    auto it = server->shops.find(shop_id);
    if (it == server->shops.end() || shop_id != client->session_vals["shop_authed"] || amount == 0)
    {
        return;
    }

    for (eodata::EIF::shop_item_t i : server->shops[shop_id].second)
    {
        if (i.id == item_id && amount <= i.max_amount)
        {
            item = i;
            uint8_t discount = 0;
            buy_price = item.buy_price;
            if (server->shop_discounts)
            {
                discount = std::round(client->character->exp / float(server->shop_discount_threshold));
                if (discount > server->shop_max_discount)
                {
                    discount = server->shop_max_discount;
                }

                buy_price = item.buy_price - std::ceil(std::round(item.buy_price * discount / 100.0));
                if (buy_price < item.sell_price)
                {
                    buy_price = item.sell_price;
                }
            }

            if (client->character->inventory->get_amount(1) >= buy_price * amount)
            {
                authed = true;
            }
            break;
        }
    }

    if (authed)
    {
        client->character->inventory->remove(1, buy_price * amount);
        client->session_vals["weight"] += server->pub.items.get(item.id).weight * amount;
        client->character->inventory->add(item.id, amount);

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SHOP, eoencoding::PACTION_BUY);
        client->get_packet_builder()->add_int(client->character->inventory->get_amount(1), 4);
        client->get_packet_builder()->add_int(item.id, 2);
        client->get_packet_builder()->add_int(amount, 4);
        client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
        client->get_packet_builder()->add_int(70, 1); // max weight
        client->Send();
    }
}

void handler::shop_sell(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t item_id = packet->get_int(2);
    size_t amount = packet->get_int(4);
    size_t shop_id = packet->get_int(4);
    bool authed = false;
    eodata::EIF::shop_item_t item;

    auto it = server->shops.find(shop_id);
    if (it == server->shops.end() || shop_id != client->session_vals["shop_authed"] || amount == 0)
    {
        return;
    }

    for (eodata::EIF::shop_item_t i : server->shops[shop_id].second)
    {
        if (i.id == item_id && client->character->inventory->get_amount(item_id) >= amount)
        {
            item = i;
            authed = true;
            break;
        }
    }

    if (authed)
    {
        client->character->inventory->add(1, item.sell_price * amount);
        client->session_vals["weight"] -= server->pub.items.get(item.id).weight * amount;
        client->character->inventory->remove(item.id, amount);

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SHOP, eoencoding::PACTION_SELL);
        client->get_packet_builder()->add_int(client->character->inventory->get_amount(item.id), 4);
        client->get_packet_builder()->add_int(item.id, 2);
        client->get_packet_builder()->add_int(client->character->inventory->get_amount(1), 4);
        client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
        client->get_packet_builder()->add_int(70, 1); // max weight
        client->Send();
    }
}
