/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "events.hpp"

namespace event
{
    void autoupdate(Server *server, EOClient *client, std::string args)
    {
#ifdef DEBUG
        out::debug("Performing database autoupdate...");
#endif // DEBUG
        bool anyonline = false;
        for (std::shared_ptr<EOClient> c : server->eoclients)
        {
            if (c->ingame)
            {
                anyonline = true;
                server->get_database()->UpdateCharacter(c->character);
            }
        }

        if (!anyonline)
        {
            /* To keep the MySQL connection alive */
            // TODO: find a better way?
            server->get_database()->generic_query("SELECT 1");
        }
    }
};
