/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

/* This is very stupid but looks a lot like on EOMain. */
inline std::string trunc_message(std::string str, size_t maxlength)
{
    std::string ret(str);
    std::string badchars = "!@#$%^&*()-=_+[]{}0123456789 ";
    size_t k = 0, l = 0;
    size_t n = str.length();
    for (size_t i = 0; i < n; ++i)
    {
        unsigned char c = str[i];
        if (c > 64 && c < 91)
        {
            ++k;
        }
    }

    for (size_t i = 0; i < n; ++i)
    {
        unsigned char c = str[i];
        if (c > 0 && c < 64)
        {
            ++l;
        }
    }

    if (n >= 100)
    {
        ret = util::strtolower(str.substr(0, k));
        for (size_t i = 0; i < badchars.length(); ++i)
        {
            ret.erase(std::remove(ret.begin(), ret.end(), badchars[i]), ret.end());
        }
        ret += str.substr(k);
    }

    return ret.substr(0, maxlength);
}

void handler::scr_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string message = trunc_message(packet->get_string(), server->max_chat_message_length);

    if (message.empty())
    {
        client->Disconnect();
        return;
    }

    if (client->character->admin > 0 && message[0] == server->admin_prefix)
    {
        std::string command = message.substr(1, -1);
        if (command.empty())
        {
            return;
        }

        std::vector<std::string> command_args = util::tokenize(command, ' ');

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->admin > 0)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_ADMIN);
                c->get_packet_builder()->add_string("server*");
                c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                c->get_packet_builder()->add_string(client->character->name + " => " + command);
                c->Send();
            }
        }

        if (command_args[0] == "x" && command_args.size() > 0 && client->character->admin >= 2)
        {
            server->ToggleHide(client);
            return;
        }

        if (command_args[0] == "k" && command_args.size() > 1 && client->character->admin >= 2)
        {
            std::vector<std::string> nicknames = util::tokenize(util::strtolower(command_args[1]), '.');
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && std::find(nicknames.begin(), nicknames.end(), c->character->name) != nicknames.end())
                {
                    server->WorldAnnounce("Attention!! " + c->character->name + " has been removed from game -" + client->character->name + " [kick]");
                    c->Disconnect();
                }
            }

            return;
        }

        if (command_args[0] == "wm" && command_args.size() > 1 && client->character->admin >= 2)
        {
            std::string nickname = util::strtolower(command_args[1]);
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->name == nickname && c->character->name != client->character->name)
                {
                    server->Warp(client, c->character->position.map, c->character->position.x, c->character->position.y, 2, true);
                    return;
                }
            }

            return;
        }

        if (command_args[0] == "st" && command_args.size() > 2 && client->character->admin >= 2)
        {
            std::string nickname = util::strtolower(command_args[1]);
            std::vector<std::string> title = command_args;
            title.erase(title.begin());
            title.erase(title.begin());
            std::string title_str = "";
            for (std::string i : title)
            {
                title_str += i + " ";
            }
            title_str.pop_back();
            title_str = util::strtolower(title_str);

            if (!util::isplaintext(title_str))
            {
                title_str = "";
            }

            if (title_str.length() > 16)
            {
                title_str = title_str.substr(0, 16);
            }

            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->name == nickname)
                {
                    c->character->title = title_str;
                    return;
                }
            }

            return;
        }

        if (command_args[0] == "sr" && command_args.size() > 2 && client->character->admin >= 2)
        {
            std::string nickname = util::strtolower(command_args[1]);
            std::string race_str = command_args[2];
            size_t race = 0;
            if (util::isnumeric(race_str))
            {
                race = stoul(race_str);
            }

            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->name == nickname)
                {
                    c->character->race = race;
                    break;
                }
            }

            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->in_range(client->character))
                {
                    handler::refresh(server, c.get());
                }
            }

            return;
        }

        if (command_args[0] == "b" && command_args.size() > 1 && client->character->admin >= 2)
        {
            size_t banlength = 120;
            if (command_args.size() > 2 && util::isnumeric(command_args[2]))
            {
                banlength = stoul(command_args[2]);
            }

            std::vector<std::string> nicknames = util::tokenize(util::strtolower(command_args[1]), '.');
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && std::find(nicknames.begin(), nicknames.end(), c->character->name) != nicknames.end())
                {
                    server->WorldAnnounce("Attention!! " + c->character->name + " has been removed from game -" + client->character->name + " [ban]");
                    bool banned = server->get_database()->AddBan(c->login, c->hdid, c->get_client()->host, banlength);
                    out::info("%d", banned);
                    c->Disconnect();
                }
            }

            return;
        }

        if (command_args[0] == "s" && client->character->admin >= 4)
        {
            server->Shutdown();
            return;
        }

        if (command_args[0] == "wa" && client->character->admin >= 4)
        {
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->id != client->id)
                {
                    server->Warp(c.get(), client->character->position.map, client->character->position.x, client->character->position.y, 2, true);
                }
            }

            return;
        }

        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->in_range(client->character, 13) && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_PLAYER);
            c->get_packet_builder()->add_int(client->id, 2);
            c->get_packet_builder()->add_string(message);
            c->Send();
        }
    }
}

void handler::prv_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string nickname = packet->get_string();
    std::string message = packet->get_string();

    if (nickname.empty() || message.empty())
    {
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->name == nickname)
        {
            if (c->character->hidden && client->character->admin == 0)
            {
                break;
            }

            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_TELL);
            c->get_packet_builder()->add_string(client->character->name);
            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            c->get_packet_builder()->add_string(trunc_message(message, server->max_chat_message_length));
            c->Send();
            return;
        }
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_int(1, 2);
    client->get_packet_builder()->add_string(nickname);
    client->Send();
}

void handler::guild_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string message = packet->get_string();

    if (client->character->guild.empty())
    {
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->guild == client->character->guild && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_REQUEST);
            c->get_packet_builder()->add_string(client->character->name);
            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            c->get_packet_builder()->add_string(message);
            c->Send();
        }
    }
}

void handler::adm_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string message = trunc_message(packet->get_string(), server->max_chat_message_length);

    if (message.empty() || client->character->admin == 0)
    {
        client->Disconnect();
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->character->admin > 0 && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_ADMIN);
            c->get_packet_builder()->add_string(client->character->name);
            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            c->get_packet_builder()->add_string(message);
            c->Send();
        }
    }
}

void handler::anc_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string message = trunc_message(packet->get_string(), server->max_chat_message_length);

    if (message.empty() || client->character->admin == 0)
    {
        client->Disconnect();
        return;
    }

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_ANNOUNCE);
            c->get_packet_builder()->add_string(client->character->name);
            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            c->get_packet_builder()->add_string(message);
            c->Send();
        }
    }
}

void handler::glb_open(Server *server, EOClient *client)
{
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_LIST);
    bool sendbuffer = false;
    for (std::pair<time_t, std::pair<std::string, std::string>> glb_line : server->glb_buffer->Get())
    {
        if (glb_line.first >= client->session_vals["glb_last_open"])
        {
            sendbuffer = true;
            client->get_packet_builder()->add_string(glb_line.second.first);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            client->get_packet_builder()->add_string(glb_line.second.second);
            client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
        }
    }

    if (sendbuffer)
    {
        client->Send();
    }

    client->session_vals["glb_last_open"] = std::time(0);
    client->glbopened = true;
}

void handler::glb_close(Server *server, EOClient *client)
{
    client->glbopened = false;
}

void handler::glb_chat(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string message = trunc_message(packet->get_string(), server->max_chat_message_length);

    server->glb_buffer->Add(client->character->name, message);

    for (std::shared_ptr<EOClient> c : server->eoclients)
    {
        if (c->ingame && c->glbopened && c->id != client->id)
        {
            c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_TALK, eoencoding::PACTION_MESSAGE);
            c->get_packet_builder()->add_string(client->character->name);
            c->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
            c->get_packet_builder()->add_string(message);
            c->Send();
        }
    }
}
