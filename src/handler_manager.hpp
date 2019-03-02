/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __HANDLER_MANAGER_HPP_INCLUDED
#define __HANDLER_MANAGER_HPP_INCLUDED

#include <memory>
#include <map>

#include "client.hpp"

class Server;
class EOClient;

class HandlerManager
{
    public:
    HandlerManager(EOClient *client);
    ~HandlerManager();
    void Register(std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type, void (*handler_ptr)(Server*, EOClient*));
    bool Unregister(std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type);
    void Execute(Server *server);

    private:
    EOClient *client;
    std::map<std::pair<eoencoding::PFamily, eoencoding::PAction>, void*> handlers;
};

#endif // __HANDLER_MANAGER_HPP_INCLUDED
