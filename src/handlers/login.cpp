/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::login(Server* server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();

    std::string login = util::strtolower(packet->get_string());
    std::string password = packet->get_string();

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_LOGIN, eoencoding::PACTION_REPLY);

    if (!server->get_database()->AccountExists(login))
    {
#ifdef DEBUG
        out::debug("[%u] Refused to login '%s': account does not exist.", client->id, login.c_str());
#endif // DEBUG
        client->get_packet_builder()->add_int(1, 2);
        client->get_packet_builder()->add_string("NO");
    }
    else
    {
        std::string password_salt;
        std::string real_password_hash;
        std::shared_ptr<sql::ResultSet> account = server->get_database()->SelectAccount(login);

        while (account->next())
        {
            password_salt = account->getString("salt");
            real_password_hash = account->getString("password");
        }

        if (util::sha256(password_salt + password + password_salt + password_salt) != real_password_hash)
        {
#ifdef DEBUG
            out::debug("[%u] Refused to login '%s': wrong password.", client->id, login.c_str());
#endif // DEBUG
            client->get_packet_builder()->add_int(2, 2);
            client->get_packet_builder()->add_string("NO");
        }
        else
        {
            if (!server->accept_logins)
            {
#ifdef DEBUG
                out::debug("[%u] Refused to login '%s': server not accepting logins.", client->id, login.c_str());
#endif // DEBUG
                client->get_packet_builder()->add_int(6, 2);
                client->get_packet_builder()->add_string("NO");
                client->Send();
                client->Disconnect();
                return;
            }

            bool loggedin = false;
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->login == login)
                {
                    loggedin = true;
                    break;
                }
            }

            if (loggedin)
            {
#ifdef DEBUG
                out::debug("[%u] Refused to login '%s': already logged in.", client->id, login.c_str());
#endif // DEBUG
                client->get_packet_builder()->add_int(5, 2);
                client->get_packet_builder()->add_string("NO");
            }
            else
            {
                if (server->get_database()->IsBanned(login))
                {
#ifdef DEBUG
                    out::debug("[%u] Refused to login '%s': account is banned.", client->id, login.c_str());
#endif // DEBUG
                    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_INIT, eoencoding::PACTION_INIT);
                    client->get_packet_builder()->add_int(2, 1);
                    client->get_packet_builder()->add_int(1, 1);
                    client->Send();
                    client->Disconnect();
                    return;
                }

                client->get_packet_builder()->add_int(3, 2);

                std::shared_ptr<sql::ResultSet> characters = server->get_database()->GetCharacters(login);
                client->get_packet_builder()->add_int(characters->rowsCount(), 1);
                client->get_packet_builder()->add_char(1);
                while (characters->next())
                {
                    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                    client->get_packet_builder()->add_string(characters->getString("name"));
                    client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);
                    client->get_packet_builder()->add_int(characters->getUInt("id"), 4);
                    client->get_packet_builder()->add_int(characters->getUInt("level"), 1);
                    client->get_packet_builder()->add_int(characters->getUInt("sex"), 1);
                    client->get_packet_builder()->add_int(characters->getUInt("hairstyle"), 1);
                    client->get_packet_builder()->add_int(characters->getUInt("haircolor"), 1);
                    client->get_packet_builder()->add_int(characters->getUInt("race"), 1);
                    client->get_packet_builder()->add_int(characters->getUInt("admin"), 1);
                    Paperdoll paperdoll(characters->getString("paperdoll"));
                    client->get_packet_builder()->add_int(server->pub.items.get(paperdoll.boots).look, 2);
                    client->get_packet_builder()->add_int(server->pub.items.get(paperdoll.armor).look, 2);
                    client->get_packet_builder()->add_int(server->pub.items.get(paperdoll.hat).look, 2);
                    client->get_packet_builder()->add_int(server->pub.items.get(paperdoll.shield).look, 2);
                    client->get_packet_builder()->add_int(server->pub.items.get(paperdoll.weapon).look, 2);
                }
                client->get_packet_builder()->add_char(eoencoding::BYTE_COMMA);

                client->login = login;
                out::info("%s has logged in", client->login.c_str());

                /* The client has logged in. Let's allow him to change the password and do other menu stuff but refuse to create an account. */
                REGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_AGREE, handler::change_password);
                REGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REQUEST, handler::request_create_character);
                REGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_CREATE, handler::create_character);
                REGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_TAKE, handler::request_delete_character);
                REGISTER_HANDLER(client, eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REMOVE, handler::delete_character);
                REGISTER_HANDLER(client, eoencoding::PFAMILY_WELCOME, eoencoding::PACTION_REQUEST, handler::request_welcome);
                UNREGISTER_HANDLER(client, eoencoding::PFAMILY_LOGIN, eoencoding::PACTION_REQUEST);
                UNREGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_REQUEST);
                UNREGISTER_HANDLER(client, eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_CREATE);
            }
        }
    }

    client->Send();
}
