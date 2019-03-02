/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::request_create_character(Server *server, EOClient *client)
{
    uint8_t characters = server->get_database()->GetCharacters(client->login)->rowsCount();
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REPLY);
    if (characters >= 3)
    {
        client->get_packet_builder()->add_int(2, 2);
        client->get_packet_builder()->add_string("NO");
    }
    else
    {
        if (client->session_vals["request_character"] == 0)
        {
            client->session_vals["request_character"] = client->random.get_int<size_t>(EO_INT2, EO_INT3 - 1);
        }
        client->get_packet_builder()->add_int(client->session_vals["request_character"], 2);
        client->get_packet_builder()->add_string("OK");
    }

    client->Send();
}

void handler::create_character(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    size_t sessid = packet->get_int(2);

    if (sessid != client->session_vals["request_character"])
    {
        /* The client is definitely not playing fair. */
        return;
    }

    uint16_t gender = packet->get_int(2);
    if (gender > 1)
    {
        return;
    }
    uint16_t hairstyle = packet->get_int(2);
    if (hairstyle < 1 || hairstyle > 20)
    {
        return;
    }
    uint16_t haircolor = packet->get_int(2);
    if (haircolor > 9)
    {
        return;
    }
    uint16_t race = packet->get_int(2);
    if (race > 3)
    {
        return;
    }
    if (packet->get_char() != eoencoding::BYTE_COMMA)
    {
        return;
    }
    std::string nickname = util::strtolower(packet->get_string());
    if (nickname.length() < 4 || nickname.length() > 12 || !util::isalpha(nickname))
    {
        return;
    }

    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REPLY);

    if (server->get_database()->CharacterExists(nickname))
    {
        client->get_packet_builder()->add_int(1, 2);
        client->get_packet_builder()->add_string("NO");
    }
    else
    {
        server->get_database()->CreateCharacter(client->login, nickname, gender, race, hairstyle, haircolor);
        client->session_vals["request_character"] = 0;
        out::info("%s has created a character '%s' (%s)", client->login.c_str(), nickname.c_str(), gender == 1? "male" : "female");

        std::shared_ptr<sql::ResultSet> characters = server->get_database()->GetCharacters(client->login);
        client->get_packet_builder()->add_int(5, 2);
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
    }

    client->Send();
}

void handler::request_delete_character(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    unsigned long id = packet->get_int(4);
    bool exists = false;

    std::shared_ptr<sql::ResultSet> characters = server->get_database()->GetCharacters(client->login);
    while (characters->next())
    {
        if (id == characters->getUInt("id"))
        {
            exists = true;
            break;
        }
    }

    if (exists)
    {
        client->session_vals["delete_character_id"] = id;
        client->session_vals["delete_character"] = client->random.get_int<size_t>(EO_INT2, EO_INT3 - 1);
        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_PLAYER);
        client->get_packet_builder()->add_int(client->session_vals["delete_character"], 2);
        client->get_packet_builder()->add_int(id, 3);
        client->Send();
    }
}

void handler::delete_character(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint16_t sessid = packet->get_int(2);
    unsigned long id = packet->get_int(4);

    if (sessid != client->session_vals["delete_character"] || id != client->session_vals["delete_character_id"])
    {
        /* Little cheater. */
        client->Disconnect();
    }

    client->session_vals["delete_character_id"] = 0;
    client->session_vals["delete_character"] = 0;

    server->get_database()->DeleteCharacter(id);

    std::shared_ptr<sql::ResultSet> characters = server->get_database()->GetCharacters(client->login);
    client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHARACTER, eoencoding::PACTION_REPLY);
    client->get_packet_builder()->add_int(6, 2);
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
    client->Send();
}
