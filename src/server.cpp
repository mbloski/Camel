/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "server.hpp"

inline std::string prepare_path(std::shared_ptr<Config> config, std::string value, std::string default_path)
{
    std::string ret = config->get_string(value);
    if (ret.empty())
    {
        out::warning("%s is not set. Defaulting to %s", value.c_str(), default_path.c_str());
        ret = default_path;
    }

    return ret;
}

Server::Server(std::shared_ptr<Config> config)
{
    bool warnings = false;

    this->config = config;
    this->eo_version = this->config->get_string("eo_version");
    this->max_chat_message_length = this->config->get_number<size_t>("max_chat_message_length");
    this->accept_new_accounts = this->config->get_bool("accept_new_accounts");
    this->accept_logins = this->config->get_bool("accept_logins");
    this->accept_characters = this->config->get_bool("accept_characters");
    this->max_players = this->config->get_number<size_t>("max_players");
    this->ping_timeout_threshold = this->config->get_number<size_t>("ping_timeout_threshold");
    this->init_patience = this->config->get_number<size_t>("init_patience");
    this->jail_map = this->config->get_number<size_t>("jail_map");
    this->arena_kill_exp = this->config->get_number<size_t>("arena_kill_exp");
    this->arena_win_exp = this->config->get_number<size_t>("arena_win_exp");
    this->arena_kill_gold = this->config->get_number<size_t>("arena_kill_gold");
    this->arena_win_gold = this->config->get_number<size_t>("arena_win_gold");
    this->idle_gold = this->config->get_number<size_t>("idle_gold");
    this->idle_gold_time = this->config->get_number<size_t>("idle_gold_time");
    this->shop_discounts = this->config->get_bool("shop_discounts");
    this->shop_discount_threshold = this->config->get_number<size_t>("shop_discount_threshold");
    this->shop_max_discount = this->config->get_number<size_t>("shop_max_discount");
    this->admin_prefix = this->config->get_string("admin_prefix")[0];
    this->database_autoupdate_delay = this->config->get_number<size_t>("database_autoupdate");
    this->glb_buffer_size = this->config->get_number<size_t>("glb_buffer_size");

    if (this->eo_version.empty())
    {
        out::warning("eo_version is not set. Defaulting to 0.0.28");
        this->eo_version = "0.0.28";
        warnings = true;
    }

    if (this->max_chat_message_length == 0)
    {
        out::warning("Warning: max_chat_message_length is set to 0");
        warnings = true;
    }

    if (!this->accept_new_accounts)
    {
        out::warning("Warning: accept_new_accounts is set to FALSE. The server will not allow to register any new accounts.");
        warnings = true;
    }

    if (!this->accept_logins)
    {
        out::warning("Warning: accept_logins is set to FALSE. The server will not accept any logins.");
        warnings = true;
    }

    if (!this->accept_characters)
    {
        out::warning("Warning: accept_characters is set to FALSE. The server will not accept any characters.");
        warnings = true;
    }

    if (this->max_players == 0)
    {
        out::warning("Warning: max_players is set to 0. No characters will be able to login.");
        warnings = true;
    }

    if (!this->config->get_bool("seqbyte_validator"))
    {
        out::warning("Warning: seqbyte_validator is set to FALSE. The server will accept packets with wrong sequence bytes.");
        warnings = true;
    }

    if (this->ping_timeout_threshold == 0)
    {
        out::warning("Warning: ping_timeout_threshold is set to 0");
        out::warning("^ This means even the alive connections will be considered dead and the server will disconnect them.");
        warnings = true;
    }

    if (this->init_patience < 5)
    {
        out::warning("Warning: init_patience is set to 0");
        warnings = true;
    }

    if (this->jail_map == 0)
    {
        out::warning("Warning: jail_map is set to 0.");
        warnings = true;
    }

    this->pub.classes = prepare_path(this->config, "ecf_file", "data/pub/dat001.ecf");
    this->pub.items = prepare_path(this->config, "eif_file", "data/pub/dat001.eif");
    this->pub.npcs = prepare_path(this->config, "enf_file", "data/pub/dtn001.enf");
    this->pub.spells = prepare_path(this->config, "esf_file", "data/pub/dsl001.esf");

    size_t map_count = this->config->get_number<size_t>("maps");
    std::string map_directory = this->config->get_string("map_directory");
    if (map_count == 0)
    {
        out::warning("Warning: maps is set to 0. Defaulting to 282.");
        map_count = 282;
        warnings = true;
    }
    if (map_directory.empty())
    {
        out::warning("Warning: map_directory is not set. Defaulting to data/maps/");
        map_directory = "data/maps/";
        warnings = true;
    }

    this->maps = new eodata::EMF(map_count, map_directory);

    for (size_t i = 1; i <= 9; ++i)
    {
        std::stringstream cvar;
        cvar << "motd" << i;
        this->motd.push_back(this->config->get_string(cvar.str()));
    }

    this->database = new Database(config);
    this->server = new Socket(config);

    /* This is so complicated because it's protected against the worst garbage.
       The config value is supposed to be in a '1/2, 3/4, 5/6' format. */
    int j = 0;
    std::vector<std::string> arena_maps = util::tokenize(this->config->get_string("arena_maps"), ',');
    for (std::string entry : arena_maps)
    {
        std::string map_id = "0";
        std::string spawn = "0";

        std::vector<std::string> i = util::tokenize(entry, '/');
        i.resize(2);
        if (util::isnumeric(i[0]))
        {
            map_id = i[0];
        }

        if (util::isnumeric(i[1]))
        {
            spawn = i[1];
        }

        ++j;
        if (map_id == "0" || spawn == "0" || map_id.empty() || spawn.empty())
        {
            warnings = true;
            out::warning("arena_maps contains wrong values. Arena %d will be disabled.", j);
        }
        else
        {
            size_t map_id_ul = stoul(map_id);
            size_t spawn_ul = stoul(spawn);

            bool duplicate = false;
            for (std::shared_ptr<Arena> arena : this->arenas)
            {
                if (arena->get_map_id() == map_id_ul)
                {
                    out::warning("Duplicate entry on map %d. Arena %d will be disabled.", map_id_ul, j);
                    duplicate = true;
                }
            }

            if (duplicate)
            {
                continue;
            }

            if (this->maps->get(map_id_ul).id == map_id_ul)
            {
                this->arenas.push_back(std::shared_ptr<Arena>(new Arena(this->maps->get(map_id_ul), spawn_ul)));
                out::info("Registered arena on map %d with a spawn rate of %d seconds.", map_id_ul, spawn_ul);
            }
            else
            {
                out::info("Couldn't register arena on map %d. Map ID is incorrect.", map_id_ul);
            }
        }
    }

    std::vector<std::string> arena_hierarchy = util::tokenize(this->config->get_string("arena_class_hierarchy"), ',');
    for (std::string entry : arena_hierarchy)
    {
        std::string class_id = "0";
        std::string exp = "0";

        std::vector<std::string> i = util::tokenize(entry, '/');
        i.resize(2);
        if (util::isnumeric(i[0]))
        {
            class_id = i[0];
        }

        if (util::isnumeric(i[1]))
        {
            exp = i[1];
        }

        size_t class_id_ul = 0;
        size_t exp_ul = 0;

        if (class_id == "0" || class_id.empty() || exp.empty())
        {
            warnings = true;
            out::warning("arena_class_hierarchy is misconfigured and will be disabled.");
            break;
        }
        else
        {
            class_id_ul = stoul(class_id);
            exp_ul = stoul(exp);
        }

        if (this->pub.classes.get(class_id_ul).id != 0)
        {
            this->arena_class_hierarchy.push_back(std::make_pair(class_id_ul, exp_ul));
        }
        else
        {
            out::warning("Ignoring non-existent class %d in arena_class_hierarchy", class_id_ul);
        }
    }

    for (size_t i = 1; i <= this->config->get_number<size_t>("shops"); ++i)
    {
        std::string shop_name = this->config->get_string("shop_" + std::to_string(i) + "_name");
        std::string raw_shop_items = this->config->get_string("shop_" + std::to_string(i) + "_items");

        std::vector<eodata::EIF::shop_item_t> newitems;

        std::vector<std::string> shop_items = util::tokenize(raw_shop_items, '|');
        for (std::string shop_item : shop_items)
        {
            std::string item_id_str = "0";
            std::string buy_price_str = "0";
            std::string sell_price_str = "0";
            std::vector<std::string> args = util::tokenize(shop_item, ',');
            args.resize(3);

            if (util::isnumeric(args[0]))
            {
                item_id_str = args[0];
            }

            if (util::isnumeric(args[1]))
            {
                buy_price_str = args[1];
            }

            if (util::isnumeric(args[2]))
            {
                sell_price_str = args[2];
            }

            if (item_id_str.empty() || buy_price_str.empty() || sell_price_str.empty())
            {
                out::error("Error loading shop entry from shop %d near \"%s\"", i, shop_item.c_str());
                continue;
            }
            else
            {
                eodata::EIF::shop_item_t newitem;
                newitem.id = stoul(item_id_str);
                newitem.buy_price = stoul(buy_price_str);
                newitem.sell_price = stoul(sell_price_str);

                newitems.push_back(newitem);
            }
        }

       this->shops[i] = std::make_pair(shop_name, newitems);
    }

    this->evman = new SchedEvents();

    j = 0;
    for (std::shared_ptr<Arena> i : this->arenas)
    {
        std::string evargv = std::to_string(j++);
        this->evman->add(SchedEvents::EventType::EVENT_REPEATABLE, i->get_spawn_rate() * 1000, event::launch_arena, nullptr, evargv);
    }

    this->evman->add(SchedEvents::EventType::EVENT_REPEATABLE, 1000, event::map_drop_effects, nullptr);

    if (this->database_autoupdate_delay > 0)
    {
        out::info("Server database will be updated automatically every %d hours.", this->database_autoupdate_delay);
        this->evman->add(SchedEvents::EventType::EVENT_REPEATABLE, 60 * 60 * this->database_autoupdate_delay, event::autoupdate, nullptr);
    }

    if (this->idle_gold > 0 && this->idle_gold_time == 0)
    {
        out::warning("idle_gold set but idle_gold_time is 0");
        this->idle_gold = 0;
        warnings = true;
    }

    if (this->shop_discounts)
    {
        if (this->shop_discount_threshold == 0)
        {
            out::warning("shop_discounts set but shop_discount_threshold is 0");
            this->shop_discounts = false;
            warnings = true;
        }

        if (this->shop_max_discount >= 100)
        {
            out::warning("shop_max_discount must be less than 100. Defaulting to 80");
            this->shop_max_discount = 80;
            warnings = true;
        }
    }

    this->glb_buffer = new GlbBuffer(this->glb_buffer_size);

    this->server->Bind();
    this->server->Listen();

    if (warnings)
    {
        for (uint8_t i = 0; i < 5; ++i)
        {
            out::warning("!!! SERVER STARTED WITH WARNINGS !!!");
        }
    }

    this->running = true;

#ifdef MULTITHREADED
    this->handle_connections_thread = std::thread(&Server::ConnectionHandler, this);
#endif // MULTITHREADED
}

void Server::run()
{
    while (this->running)
    {
#ifndef MULTITHREADED
        this->HandleNewConnections();
#else
        this->server->Select(Socket::SELECT_READ);
#endif // MULTITHREADED

        for (std::shared_ptr<EOClient> c : this->eoclients)
        {
            if (!this->running)
            {
                break;
            }
            
            std::string packet = c->Tick();
            if (!packet.empty())
            {
                c->get_handler_manager()->Execute(this);
            }
        }

        this->evman->Tick(this);

#ifndef MULTITHREADED
        /* Cleanup clients marked as disconnected. */
        this->CleanupConnections();
#endif // MULTITHREADED
    }
}

/** Handles any incoming connections and accepts them (or not...), produces instances of "EOClient". */
void Server::HandleNewConnections()
{
    Client *newconnection = this->server->HandleNew();
    if (newconnection != nullptr)
    {
        this->eoclients.push_back(std::shared_ptr<EOClient>(new EOClient(config, newconnection)));
    }
}

void Server::CleanupConnections()
{
    for (std::vector<std::shared_ptr<EOClient>>::iterator it = this->eoclients.begin(); it != this->eoclients.end(); )
    {
        std::shared_ptr<EOClient> eoclient = *it;
        if (!eoclient->get_client()->is_connected() || (eoclient->initialized && eoclient->missed_pings > this->ping_timeout_threshold) || (!eoclient->initialized && eoclient->get_age() >= this->init_patience))
        {
            if (!eoclient->initialized && eoclient->get_age() >= this->init_patience)
            {
                eoclient->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);
                eoclient->get_packet_builder()->add_char(255);
                eoclient->get_packet_builder()->add_string("Endless Online v" + this->eo_version + "\r\n");
                eoclient->Send();
            }

            out::info("Client connecting from %s disconnected.", this->server->addr_to_string(eoclient->get_client()->host).c_str());

            if (eoclient->ingame)
            {
                --this->players_ingame;
                for (std::shared_ptr<EOClient> c : this->eoclients)
                {
                    if (c->ingame && c->character->in_range(eoclient->character) && c->id != eoclient->id)
                    {
                        c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_PLAYERS, eoencoding::PACTION_REMOVE);
                        c->get_packet_builder()->add_int(eoclient->id, 2);
                        c->Send();
                    }
                }

                if (eoclient->onarena)
                {
                    eoclient->onarena = false;
                    for (std::shared_ptr<Arena> arena : this->arenas)
                    {
                        arena->Cleanup(this);
                    }
                }

                this->evman->cleanup(eoclient.get());
                this->database->UpdateCharacter(eoclient->character);
            }

            /* We're only getting rid of the inactive instances of EOClient in this loop!
               Let's close the actual connection. */
            this->server->Close(eoclient->get_client());
            /* Add its player ID to id_cache to reuse. */
            if (eoclient->id != 0)
            {
                this->id_cache.push_back(eoclient->id);
            }
            /* Forget about the client. */
            it = this->eoclients.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void Server::WorldAnnounce(std::string str)
{
    for (std::shared_ptr<EOClient> c : this->eoclients)
    {
        if (c->ingame)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_SERVER);
            c->get_packet_builder()->add_string(str);
            c->Send();
        }
    }
}

void Server::MapAnnounce(size_t id, std::string str)
{
    for (std::shared_ptr<EOClient> c : this->eoclients)
    {
        if (c->ingame && c->character->position.map == id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_SERVER);
            c->get_packet_builder()->add_string(str);
            c->Send();
        }
    }
}

void Server::Warp(EOClient *client, uint16_t map, uint8_t x, uint8_t y, uint8_t anim, bool forced)
{
    client->session_vals["warp_secid"] = this->random.get_int(EO_INT2, EO_INT3 - 1);
    client->session_vals["warp_tomap"] = map;
    client->session_vals["warp_tox"] = x;
    client->session_vals["warp_anim"] = anim;
    client->session_vals["warp_toy"] = y;
    client->session_vals["warp_forced"] = forced? 1 : 0;
    client->session_vals["warp_forced_p2"] = 0;
    uint8_t warptype = client->character->position.map == map? 1 : 2;
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_WARP, eoencoding::PACTION_REQUEST);
    client->get_packet_builder()->add_int(warptype, 1);
    client->get_packet_builder()->add_int(map, 2);
    if (warptype == 1)
    {
        client->get_packet_builder()->add_int(x, 1);
        client->get_packet_builder()->add_int(y, 1);
    }
    else
    {
        client->get_packet_builder()->add_string(this->maps->get(map).checksum);
        client->get_packet_builder()->add_int(client->session_vals["warp_secid"], 2);
    }

    client->Send();

    if (forced)
    {
        handler::warp(this, client);
    }
}

void Server::AddExp(EOClient *client, size_t exp)
{
    if (!client->ingame)
        return;

    uint8_t levelbyte = 0;
    uint8_t old_level = client->character->level;
    client->character->exp += exp;
    client->character->UpdateStats();

    if (old_level < client->character->level)
    {
        levelbyte = client->character->level;

        /* since this is a hacky way to add exp, we have to use a level-up emote without the sound */
        for (std::shared_ptr<EOClient> c : this->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_EMOTE, eoencoding::PACTION_PLAYER);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(13, 1);
                c->Send();
            }
        }
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_int(6, 1); // exp scroll
    client->get_packet_builder()->add_int(0, 2); // item id - we didn't use any
    client->get_packet_builder()->add_int(0, 4); // item amount
    client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
    client->get_packet_builder()->add_int(70, 1); // max weight
    client->get_packet_builder()->add_int(client->character->exp, 4);
    client->get_packet_builder()->add_int(levelbyte, 1);
    client->get_packet_builder()->add_int(0, 2); // stat points
    client->get_packet_builder()->add_int(0, 2); // skill points
    client->get_packet_builder()->add_int(client->character->hp, 2); // max hp
    client->get_packet_builder()->add_int(client->character->tp, 2); // max tp
    client->get_packet_builder()->add_int(20, 2); // sp
    client->Send();
}

void Server::AddItem(EOClient *client, uint16_t item, size_t amount, uint16_t drop_id)
{
    client->character->inventory->add(item, amount);
    client->session_vals["weight"] += this->pub.items.get(item).weight * amount;

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ITEM, eoencoding::PACTION_GET);
    client->get_packet_builder()->add_int(drop_id, 2);
    client->get_packet_builder()->add_int(item, 2);
    client->get_packet_builder()->add_int(amount, 3);
    client->get_packet_builder()->add_int(client->session_vals["weight"], 1);
    client->get_packet_builder()->add_int(70, 1); // max weight
    client->Send();
}

void Server::ToggleHide(EOClient *client)
{
    if (!client->ingame)
    {
        return;
    }

    if (!client->character->hidden)
    {
        client->character->hidden = true;
        for (std::shared_ptr<EOClient> c : this->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character))
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ADMINITERACT, eoencoding::PACTION_REMOVE);
                c->get_packet_builder()->add_int(client->id, 2);
                c->Send();
            }
        }
    }
    else
    {
        client->character->hidden = false;
        for (std::shared_ptr<EOClient> c : this->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character))
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ADMINITERACT, eoencoding::PACTION_AGREE);
                c->get_packet_builder()->add_int(client->id, 2);
                c->Send();
            }
        }
    }
}

void Server::Shutdown()
{
    out::info("closing server connections...");
    for (std::shared_ptr<EOClient> c : this->eoclients)
    {
        if (c->ingame)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_MESSAGE, eoencoding::PACTION_CLOSE);
            c->Send();
        }

        c->Disconnect();
    }

    out::info("saving gamestate...");
    this->CleanupConnections();
    this->running = false;
}

#ifdef MULTITHREADED
void Server::ConnectionHandler()
{
    while (this->running)
    {
        this->HandleNewConnections();
        this->CleanupConnections();
    }
}
#endif // MULTITHREADED

Server::~Server()
{
#ifdef MULTITHREADED
    this->handle_connections_thread.join();
#endif // MULTITHREADED
    delete this->database;
    delete this->server;
    delete this->maps;
    delete this->evman;
    delete this->glb_buffer;
    out::info("Server halted");
}
