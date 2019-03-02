/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"
#include "helpers.hpp"

inline void register_game_handlers(EOClient *client)
{
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_REQUEST, handler::online_list);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_LIST, handler::online_list_friends);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_ACCEPT, handler::find_player);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_MESSAGE, eoencoding::PACTION_PING, handler::latency);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_REPORT, handler::scr_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_TELL, handler::prv_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_REQUEST, handler::guild_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_ADMIN, handler::adm_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_MESSAGE, handler::glb_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_TALK, eoencoding::PACTION_ANNOUNCE, handler::anc_chat);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_GLOBAL, eoencoding::PACTION_OPEN, handler::glb_open);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_GLOBAL, eoencoding::PACTION_CLOSE, handler::glb_close);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_EMOTE, eoencoding::PACTION_REPORT, handler::emote);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_FACE, eoencoding::PACTION_PLAYER, handler::face);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_SIT, eoencoding::PACTION_REQUEST, handler::sit);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_CHAIR, eoencoding::PACTION_REQUEST, handler::chair);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_WALK, eoencoding::PACTION_PLAYER, handler::walk);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_WALK, eoencoding::PACTION_SPEC, handler::walk);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_WALK, eoencoding::PACTION_ADMIN, handler::walk);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ATTACK, eoencoding::PACTION_USE, handler::attack);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_REFRESH, eoencoding::PACTION_REQUEST, handler::refresh);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_APPEAR, eoencoding::PACTION_REQUEST, handler::appear_approach);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_AUTOREFRESH, eoencoding::PACTION_REQUEST, handler::appear_approach);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_REQUEST, handler::open_paperdoll);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_ADD, handler::equip);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_PAPERDOLL, eoencoding::PACTION_REMOVE, handler::unequip);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_BOOK, eoencoding::PACTION_REQUEST, handler::open_book);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ITEM, eoencoding::PACTION_JUNK, handler::junk);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ITEM, eoencoding::PACTION_DROP, handler::drop);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_ITEM, eoencoding::PACTION_GET, handler::pick);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_WARP, eoencoding::PACTION_TAKE, handler::warp_nomap);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_WARP, eoencoding::PACTION_ACCEPT, handler::warp);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_DOOR, eoencoding::PACTION_OPEN, handler::door);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_QUEST, eoencoding::PACTION_LIST, handler::open_questlist);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_QUEST, eoencoding::PACTION_ACCEPT, handler::take_quest);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_SHOP, eoencoding::PACTION_BUY, handler::shop_buy);
    REGISTER_HANDLER(client, eoencoding::PFAMILY_SHOP, eoencoding::PACTION_SELL, handler::shop_sell);
}

void handler::request_welcome(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    unsigned long id = packet->get_int(4);

    std::shared_ptr<sql::ResultSet> characters = server->get_database()->GetCharacters(client->login);
    while (characters->next())
    {
        /* Only react if a character with the requested ID exists on player's account. Call me paranoid. */
        if (characters->getUInt("id") == id)
        {
            client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_REPLY);
            if (!server->accept_characters || server->get_player_count() >= server->max_players)
            {
                client->get_packet_builder()->add_int(3, 2);
                client->get_packet_builder()->add_string("NO");
                client->Send();
                return;
            }

            client->character = new Character(characters);
            out::info("Character %s selected to login.", client->character->name.c_str());

            if (!client->character->guild.empty())
            {
                /* Establish a guild. */
                std::shared_ptr<sql::ResultSet> guild = server->get_database()->GetGuild(client->character->guild);
                if (guild->rowsCount() == 0)
                {
                    /* If the guild with character's tag existed, it does not anymore. Cleanup. */
                    client->character->guild = "";
                    client->character->guild_rank = 0;
                    client->character->guild_rank_str = "";
                }
                else
                {
                    while (guild->next())
                    {
                        if (client->character->guild_rank > 9 || client->character->guild_rank < 1)
                        {
                            client->character->guild_rank = 9;
                        }
                        client->character->guild_name = guild->getString("name");
                    }
                }
            }

            if (server->maps->get(client->character->position.map).id == 0)
            {
                out::warning("Character %s is on a non-existent map!!! Disconnecting the client.", client->character->name.c_str());
                client->Disconnect();
                return;
            }

            /* If the map enforces relog coords, let's correct them now. */
            if (server->maps->get(client->character->position.map).relogx != 0 || server->maps->get(client->character->position.map).relogy != 0)
            {
                client->character->position.x = server->maps->get(client->character->position.map).relogx;
                client->character->position.y = server->maps->get(client->character->position.map).relogy;
                client->character->sitting = 0;
            }

            client->get_packet_builder()->add_int(1, 2);
            client->get_packet_builder()->add_int(client->id, 2);
            client->get_packet_builder()->add_int(id, 4);
            client->get_packet_builder()->add_int(client->character->position.map, 2);
            client->get_packet_builder()->add_string(server->maps->get(client->character->position.map).checksum);
            client->get_packet_builder()->add_string(server->pub.items.get_checksum());
            client->get_packet_builder()->add_string(server->pub.npcs.get_checksum());
            client->get_packet_builder()->add_string(server->pub.spells.get_checksum());
            client->get_packet_builder()->add_string(server->pub.classes.get_checksum());
            client->get_packet_builder()->add_string(client->character->name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(client->character->title);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(client->character->guild_name);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(client->character->guild_rank_str);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_int(client->character->class_id, 1);
            client->get_packet_builder()->add_string(util::str_pad(client->character->guild, 3, ' ', util::STR_PAD_RIGHT));
            client->get_packet_builder()->add_int(client->character->admin, 1);
            client->get_packet_builder()->add_int(client->character->level, 1);
            client->get_packet_builder()->add_int(client->character->exp, 4);
            client->get_packet_builder()->add_int(client->character->usage, 4);
            client->get_packet_builder()->add_int(client->character->hp, 2);
            client->get_packet_builder()->add_int(client->character->hp, 2); // max hp
            client->get_packet_builder()->add_int(client->character->tp, 2);
            client->get_packet_builder()->add_int(client->character->tp, 2); // max hp
            client->get_packet_builder()->add_int(20, 2); // SP
            client->get_packet_builder()->add_int(0, 2); // ST points
            client->get_packet_builder()->add_int(0, 2); // SK points
            client->get_packet_builder()->add_int(client->character->karma, 2);
            client->get_packet_builder()->add_int(0, 2); // min damage
            client->get_packet_builder()->add_int(0, 2); // max damage
            client->get_packet_builder()->add_int(0, 2); // accu
            client->get_packet_builder()->add_int(0, 2); // eva
            client->get_packet_builder()->add_int(0, 2); // defense
            client->get_packet_builder()->add_int(client->character->stats.str, 2);
            client->get_packet_builder()->add_int(client->character->stats.intelligence, 2);
            client->get_packet_builder()->add_int(client->character->stats.wis, 2);
            client->get_packet_builder()->add_int(client->character->stats.agi, 2);
            client->get_packet_builder()->add_int(client->character->stats.con, 2);
            client->get_packet_builder()->add_int(client->character->stats.cha, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->boots, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->gloves, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->gem, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->armor, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->belt, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->necklace, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->hat, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->shield, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->weapon, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->rings.first, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->rings.second, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->bracelets.first, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->bracelets.second, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->bracers.first, 2);
            client->get_packet_builder()->add_int(client->character->paperdoll->bracers.second, 2);
            client->get_packet_builder()->add_int(client->character->guild_rank, 1);
            client->get_packet_builder()->add_int(server->jail_map, 2);
            client->get_packet_builder()->add_int(4, 2);
            client->get_packet_builder()->add_int(24, 1);
            client->get_packet_builder()->add_int(24, 1);
            client->get_packet_builder()->add_int(10, 2);
            client->get_packet_builder()->add_int(10, 2);
            client->get_packet_builder()->add_int(1, 2);
            client->get_packet_builder()->add_int(1, 2);
            client->get_packet_builder()->add_int((client->character->usage == 0)? 250 : 0, 1);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->Send();

            REGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_AGREE, handler::send_file);
            REGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_MESSAGE, handler::welcome);
            break;
        }
    }
}

void handler::send_file(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t action = packet->get_int(1);
    uint16_t pid = packet->get_int(2);

    if (pid != client->id)
    {
        client->Disconnect();
        return;
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);

    switch (action)
    {
        case 1: // EMF
        {
            size_t mapid = packet->get_int(2);
            if (mapid != 0)
            {
                out::info("Sending file %s to client %u", server->maps->get(mapid).path.c_str(), client->id);
                client->get_packet_builder()->add_int(4, 1);
                client->get_packet_builder()->add_string(eodata::get_file(server->maps->get(mapid).path.c_str()));
            }
            else
            {
                return;
            }
            break;
        }
        case 2: // EIF
            out::info("Sending file %s to client %u", server->pub.items.get_filename().c_str(), client->id);
            client->get_packet_builder()->add_int(5, 1);
            client->get_packet_builder()->add_int(1, 1); // file ID. The official client only supports 1.
            client->get_packet_builder()->add_string(server->pub.items.get_file());
        break;
        case 3: // ENF
            out::info("Sending file %s to client %u", server->pub.npcs.get_filename().c_str(), client->id);
            client->get_packet_builder()->add_int(6, 1);
            client->get_packet_builder()->add_int(1, 1);
            client->get_packet_builder()->add_string(server->pub.npcs.get_file());
        break;
        case 4: // ESF
            out::info("Sending file %s to client %u", server->pub.spells.get_filename().c_str(), client->id);
            client->get_packet_builder()->add_int(7, 1);
            client->get_packet_builder()->add_int(1, 1);
            client->get_packet_builder()->add_string(server->pub.spells.get_file());
        break;
        case 5:  // ECF
            out::info("Sending file %s to client %u", server->pub.classes.get_filename().c_str(), client->id);
            client->get_packet_builder()->add_int(11, 1);
            client->get_packet_builder()->add_int(1, 1);
            client->get_packet_builder()->add_string(server->pub.classes.get_file());
        break;
    }

    client->Send();
}

void handler::welcome(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    size_t pid = packet->get_int(3);
    size_t charid = packet->get_int(4);

    if (pid != client->id || charid != client->character->id)
    {
        client->Disconnect();
        return;
    }

    client->ingame = true;
    server->inc_player_count();

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_int(2, 2);

    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    for (std::string motdline : server->motd)
    {
        client->get_packet_builder()->add_string(motdline);
        client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    }

    client->session_vals["weight"] = 0;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->weapon).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->shield).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->armor).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->hat).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->boots).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->gloves).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->gem).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->belt).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->necklace).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->rings.first).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->rings.second).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->bracelets.first).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->bracelets.second).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->bracers.first).weight;
    client->session_vals["weight"] += server->pub.items.get(client->character->paperdoll->bracers.second).weight;
    for (std::pair<size_t, size_t> i : client->character->inventory->get())
    {
        client->session_vals["weight"] += server->pub.items.get(i.first).weight * i.second;
    }
    client->session_vals["weight"] = client->session_vals["weight"] > 250? 250 : client->session_vals["weight"];

    client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
    client->get_packet_builder()->add_int(70, 1); // max weight

    std::map<size_t, size_t> inventory = client->character->inventory->get();
    for (std::map<size_t, size_t>::iterator it = inventory.begin(); it != inventory.end(); ++it)
    {
        client->get_packet_builder()->add_int(it->first, 2);
        client->get_packet_builder()->add_int(it->second, 4);
    }
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    // spells here
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    size_t in_range = 0;
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && client->character->in_range(c->character))
        {
            ++in_range;
        }
    }
    client->get_packet_builder()->add_int(in_range, 1);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && client->character->in_range(c->character))
        {
            add_player_to_packet_builder(server, client, c.get());
        }
    }
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    // npcs here
    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
    add_item_drops_in_range_to_packet_builder(server, client);
    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character) && client->id != c->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_AGREE);
            add_player_to_packet_builder(server, c.get(), client);
            c->Send();
        }
    }

    client->Send();

    if (server->idle_gold > 0)
    {
        server->evman->add(SchedEvents::EVENT_REPEATABLE, server->idle_gold_time * 60000, [](Server *server, EOClient *client, std::string){server->AddItem(client, 1, server->idle_gold);}, client);
    }

    /* Game state change. We're in game. Let's unregister all the menu things. */
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_AGREE);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REQUEST);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_CREATE);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_TAKE);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REMOVE);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_REQUEST);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_AGREE);
    UNREGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_MESSAGE);
    /* ...and register the game things. */
    register_game_handlers(client);
}
