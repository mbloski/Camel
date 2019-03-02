/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

void handler::open_questlist(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t action = packet->get_int(1);

    if (action != 1)
    {
        return;
    }

    /* It's a dummy shop system for now. */
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_QUEST, eoencoding::PACTION_DIALOG);
    client->get_packet_builder()->add_int(1, 2); // switch between quests. Useless here.
    client->get_packet_builder()->add_int(0, 2); // vendor id? Useless in this case
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    client->get_packet_builder()->add_int(0, 2); // ?
    client->get_packet_builder()->add_string("dummy menu");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);

    client->get_packet_builder()->add_int(1, 2); // type "line"
    client->get_packet_builder()->add_string("Please select a shop or click OK to open your book.");
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    for (std::pair<size_t, std::pair<std::string, std::vector<eodata::EIF::shop_item_t>>> i : server->shops)
    {
        client->get_packet_builder()->add_int(2, 2); // type "link"
        client->get_packet_builder()->add_int(i.first, 2);
        client->get_packet_builder()->add_string(i.second.first);
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    }
    client->Send();
}

void handler::take_quest(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    packet->get_int(2); // ?
    packet->get_int(2); // ?
    packet->get_int(2); // ?
    packet->get_int(2); // ?
    uint8_t action = packet->get_int(1);
    uint8_t id = packet->get_int(1);

    if (action == 1 && id == 0)
    {
        send_book(client, client);
    }
    else if (action == 2)
    {
        auto it = server->shops.find(id);
        if (it == server->shops.end())
        {
            return;
        }

        std::string shop_name = server->shops[id].first;
        size_t discount = 0;
        if (server->shop_discounts)
        {
            discount = std::round(client->character->exp / float(server->shop_discount_threshold));
            if (discount > server->shop_max_discount)
            {
                discount = server->shop_max_discount;
            }

            shop_name += " (" + std::to_string(discount) + "% discount)";
        }

        client->session_vals["shop_authed"] = id;
        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SHOP, eoencoding::PACTION_OPEN);
        client->get_packet_builder()->add_int(id, 2);
        client->get_packet_builder()->add_string(shop_name);
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        for (eodata::EIF::shop_item_t item : server->shops[id].second)
        {
            size_t buy_price = item.buy_price;
            if (server->shop_discounts)
            {
                buy_price = item.buy_price - std::ceil(std::round(item.buy_price * discount / 100.0));
                if (buy_price < item.sell_price)
                {
                    buy_price = item.sell_price;
                }
            }
            client->get_packet_builder()->add_int(item.id, 2);
            client->get_packet_builder()->add_int(buy_price, 3);
            client->get_packet_builder()->add_int(item.sell_price, 3);
            client->get_packet_builder()->add_int(item.max_amount, 1);
        }
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        client->Send();
    }
}
