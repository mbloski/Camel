/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::name_availability(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string name = packet->get_string();

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_REPLY);

    if (!server->accept_new_accounts)
    {
        client->get_packet_builder()->add_int(7, 2);
        client->get_packet_builder()->add_string("NO");
        client->Send();
        client->Disconnect();
        return;
    }

    if (!server->get_database()->AccountExists(name))
    {
        size_t accreply_time = util::get_eo_timestamp();
        client->get_packet_builder()->add_int(accreply_time, 3);
        client->get_packet_builder()->add_string("OK");
    }
    else
    {
        client->get_packet_builder()->add_int(1, 2);
        client->get_packet_builder()->add_string("NO");
    }

    client->Send();
}

void handler::register_account(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    /*size_t sess =*/ packet->get_int(2);
    if (packet->get_char() != eoencoding::BYTE_COMMA)
    {
        return;
    }
    std::string name = packet->get_string();
    std::string password = packet->get_string();
    std::string realname = packet->get_string();
    std::string location = packet->get_string();
    std::string email = packet->get_string();
    std::string pcname = packet->get_string();
    size_t hdid = std::stoull(packet->get_string());

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_REPLY);

    if (!server->accept_new_accounts)
    {
        return;
    }

    if (name.length() > 16 || realname.length() > 64 || location.length() > 64 || email.length() > 64)
    {
        return;
    }

    if (server->get_database()->AccountExists(name))
    {
        return;
    }

    server->get_database()->CreateAccount(name, password, realname, location, email, pcname, hdid);
    out::info("Client %u has registered a new account: '%s'", client->id, name.c_str());

    client->get_packet_builder()->add_int(3, 2);
    client->get_packet_builder()->add_string("GO");
    client->Send();
}

void handler::change_password(Server* server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::string login = util::strtolower(packet->get_string());
    std::string oldpassword = packet->get_string();
    std::string newpassword = packet->get_string();

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_ACCOUNT, eoencoding::PACTION_REPLY);

    std::string oldpassword_salt;
    std::string oldpassword_hash;
    std::shared_ptr<sql::ResultSet> account = server->get_database()->SelectAccount(login);
    while (account->next())
    {
        oldpassword_salt = account->getString("salt");
        oldpassword_hash = account->getString("password");
    }

    if (client->login == login && util::sha256(oldpassword_salt + oldpassword + oldpassword_salt + oldpassword_salt) == oldpassword_hash)
    {
        server->get_database()->SetPassword(login, newpassword);

#ifdef DEBUG
        out::debug("The password for username '%s' has been changed.", client->login.c_str());
#endif // DEBUG

        client->get_packet_builder()->add_int(6, 2);
        client->get_packet_builder()->add_string("OK");
    }
    else
    {
        client->get_packet_builder()->add_int(5, 2);
        client->get_packet_builder()->add_string("NO");
    }

    client->Send();
}
