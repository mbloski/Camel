/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "events.hpp"

namespace event
{
    void map_drop_effects(Server *server, EOClient *client, std::string args)
    {

        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            bool effects = false;
            std::vector<std::pair<uint8_t, std::pair<uint8_t, uint8_t>>> effects_to_send;
            if (!c->ingame)
            {
                continue;
            }

            for (std::pair<size_t, eodata::EMF::drop_t> i : server->maps->get(c->character->position.map).drops)
            {
                eodata::EMF::drop_t drop = i.second;

                if (drop.archived)
                {
                    continue;
                }

                if (drop.effect != 0 && c->character->in_range_of(c->character->position.map, drop.pos.first, drop.pos.second))
                {
                    effects = true;
                    effects_to_send.push_back(std::make_pair(drop.effect, std::make_pair(drop.pos.first, drop.pos.second)));
                }
            }

            if (effects)
            {
                c->get_packet_builder()->PreparePacket(eoencoding::PFAMILY_EFFECT, eoencoding::PACTION_AGREE);
                for (std::pair<uint8_t, std::pair<uint8_t, uint8_t>> i : effects_to_send)
                {
                    c->get_packet_builder()->add_int(i.second.first, 1);
                    c->get_packet_builder()->add_int(i.second.second, 1);
                    c->get_packet_builder()->add_int(i.first, 2);
                }
                c->Send();
            }
        }

    }
};
