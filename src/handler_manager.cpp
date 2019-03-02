/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "handler_manager.hpp"

HandlerManager::HandlerManager(EOClient *client)
{
    this->client = client;
}

HandlerManager::~HandlerManager()
{

}

void HandlerManager::Register(std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type, void (*handler_ptr)(Server*, EOClient*))
{
    this->handlers.insert(std::make_pair(packet_type, (void*)handler_ptr));
#ifdef DEBUG
    if (this->client->id == 0)
    {
        out::debug("Registered handler {%u_%u => %p} for uninitialized client.", (uint8_t)packet_type.second, (uint8_t)packet_type.first, handler_ptr);
    }
    else
    {
        out::debug("Registered handler {%u_%u => %p} for client %u.", (uint8_t)packet_type.second, (uint8_t)packet_type.first, handler_ptr, this->client->id);
    }
#endif // DEBUG
}

bool HandlerManager::Unregister(std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type)
{
    if (this->handlers.count(packet_type) != 0)
    {
        this->handlers.erase(packet_type);
#ifdef DEBUG
        out::debug("Unregistered handler %u_%u for client %u.", (uint8_t)packet_type.second, (uint8_t)packet_type.first, this->client->id);
#endif // DEBUG
        return true;
    }

    return false;
}

void HandlerManager::Execute(Server* server)
{
    std::shared_ptr<PacketReader> packet = client->get_packet_reader();
    std::pair<eoencoding::PFamily, eoencoding::PAction> packet_type = std::make_pair(packet->get_family(), packet->get_action());

    void *handler_ptr = this->handlers[packet_type];

    if (handler_ptr != nullptr)
    {
        void (*f)(Server*, EOClient*) = (void(*)(Server*, EOClient*))handler_ptr;
        (*f)(server, this->client);
    }
    else
    {
#ifdef DEBUG
    if (this->client->id == 0)
    {
        out::debug("Received unhandled packet: %u_%u from uninitialized client", packet_type.second, packet_type.first);
    }
    else
    {
        out::debug("Received unhandled packet: %u_%u from client %u", packet_type.second, packet_type.first, this->client->id);
    }
#endif // DEBUG
    }

    return;
}
