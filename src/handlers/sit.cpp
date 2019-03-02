/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handlers.hpp"

void handler::sit(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t action = packet->get_int(1);

    if (client->character->sitting == 0 && action == 1)
    {
        client->character->sitting = 2;

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SIT, eoencoding::PACTION_REPLY);
        client->get_packet_builder()->add_int(client->id, 2);
        client->get_packet_builder()->add_int(client->character->position.x, 1);
        client->get_packet_builder()->add_int(client->character->position.y, 1);
        client->get_packet_builder()->add_int(client->character->position.direction, 1);
        client->get_packet_builder()->add_int(0, 1);
        client->Send();

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && client->character->in_range(c->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SIT, eoencoding::PACTION_PLAYER);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(client->character->position.x, 1);
                c->get_packet_builder()->add_int(client->character->position.y, 1);
                c->get_packet_builder()->add_int(client->character->position.direction, 1);
                c->get_packet_builder()->add_int(0, 1);
                c->Send();
            }
        }
    }
    else if (client->character->sitting == 2 && action == 2)
    {
        client->character->sitting = 0;

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SIT, eoencoding::PACTION_CLOSE);
        client->get_packet_builder()->add_int(client->id, 2);
        client->get_packet_builder()->add_int(client->character->position.x, 1);
        client->get_packet_builder()->add_int(client->character->position.y, 1);
        client->Send();

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && client->character->in_range(c->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SIT, eoencoding::PACTION_REMOVE);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(client->character->position.x, 1);
                c->get_packet_builder()->add_int(client->character->position.y, 1);
                c->Send();
            }
        }
    }
}

void handler::chair(Server *server, EOClient *client)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    uint8_t action = packet->get_int(1);

    if (action == 2 && client->character->sitting == 1) // stand up
    {
        switch (client->character->position.direction)
        {
            case 0:
            ++client->character->position.y;
            break;
            case 1:
            --client->character->position.x;
            break;
            case 2:
            --client->character->position.y;
            break;
            case 3:
            ++client->character->position.x;
            break;
        }
        client->character->sitting = 0;

        client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHAIR, eoencoding::PACTION_CLOSE);
        client->get_packet_builder()->add_int(client->id, 2);
        client->get_packet_builder()->add_int(client->character->position.x, 1);
        client->get_packet_builder()->add_int(client->character->position.y, 1);
        client->Send();

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_SIT, eoencoding::PACTION_REMOVE);
                c->get_packet_builder()->add_int(client->id, 2);
                c->get_packet_builder()->add_int(client->character->position.x, 1);
                c->get_packet_builder()->add_int(client->character->position.y, 1);
                c->Send();
            }
        }

        return;
    }

    uint8_t x = packet->get_int(1);
    uint8_t y = packet->get_int(1);

    uint8_t reliable_direction = client->character->position.x - x + ((client->character->position.y - y == 0)? client->character->position.y - y + 1 : client->character->position.y - y) + 1;

    if (reliable_direction > 3)
    {
        return;
    }

    std::vector<eodata::EMF::tile_t> avail_chairtypes;
    avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_ALL);

    uint8_t chair_direction;

    switch (reliable_direction)
    {
        case 0: // UP
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_UP);
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_UP_LEFT);
        chair_direction = 2;
        break;
        case 1: // LEFT
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_LEFT);
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_UP_LEFT);
        chair_direction = 1;
        break;
        case 2: // DOWN
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_DOWN);
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_DOWN_RIGHT);
        chair_direction = 0;
        break;
        case 3: // RIGHT
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_RIGHT);
        avail_chairtypes.push_back(eodata::EMF::TILE_CHAIR_DOWN_RIGHT);
        chair_direction = 3;
        break;
    }

    for (eodata::EMF::tile_t chairtype : avail_chairtypes)
    {
        if (server->maps->get(client->character->position.map).tiles[std::make_pair(x, y)].type == chairtype)
        {
            for (std::shared_ptr<EOClient> c : server->eoclients)
            {
                if (c->ingame && c->character->in_range(client->character) && c->id != client->id)
                {
                    if (x == c->character->position.x && y == c->character->position.y)
                    {
                        /* The chair is busy. */
                        return;
                    }
                    c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHAIR, eoencoding::PACTION_PLAYER);
                    c->get_packet_builder()->add_int(client->id, 2);
                    c->get_packet_builder()->add_int(x, 1);
                    c->get_packet_builder()->add_int(y, 1);
                    c->get_packet_builder()->add_int(chair_direction, 1);
                    c->Send();
                }
            }

            client->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_CHAIR, eoencoding::PACTION_REPLY);
            client->get_packet_builder()->add_int(client->id, 2);
            client->get_packet_builder()->add_int(x, 1);
            client->get_packet_builder()->add_int(y, 1);
            client->get_packet_builder()->add_int(chair_direction, 1);
            client->Send();

            client->character->position.x = x;
            client->character->position.y = y;
            client->character->position.direction = chair_direction;
            client->character->sitting = 1;
            return;
        }
    }
}
