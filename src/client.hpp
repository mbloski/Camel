/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __CLIENT_HPP_INCLUDED
#define __CLIENT_HPP_INCLUDED

#include <netdb.h>

#include "socket.hpp"
#include "packet_builder.hpp"
#include "packet_reader.hpp"
#include "character.hpp"
#include "handler_manager.hpp"
#include "handlers/handlers.hpp"

class HandlerManager;

class EOClient
{
    public:
    EOClient(std::shared_ptr<Config> config, Client *client);
    ~EOClient();
    Client *get_client() const { return this->client; }
    std::shared_ptr<HandlerManager> get_handler_manager() const { return this->handler_manager; }
    std::shared_ptr<PacketBuilder> get_packet_builder() const { return this->packet_builder; }
    std::shared_ptr<PacketReader> get_packet_reader() const { return this->packet_reader; }
    std::pair<uint8_t, uint8_t> get_multi() const { return std::make_pair(this->packet_builder->get_multi(), this->packet_reader->get_multi()); }
    time_t get_age() { return std::time(0) - this->alive_since; }

    bool Send();
    std::string Tick();
    void Disconnect();

    util::rand random;
    bool sequence_validator;
    size_t missed_pings = 0;
    bool initialized = false;
    bool ingame = false;
    bool onarena = false;
    bool glbopened = false;

    Character *character = 0;

    size_t ping_delay;
    size_t id = 0;
    size_t hdid;

    std::string login;

    std::map<std::string, size_t> session_vals;

    private:
    std::string Recv();

    time_t alive_since;
    time_t last_ping_time;
    std::shared_ptr<Config> config;
    Client *client = 0;
    std::shared_ptr<HandlerManager> handler_manager;
    std::shared_ptr<PacketBuilder> packet_builder;
    std::shared_ptr<PacketReader> packet_reader;
};

#endif // __CLIENT_HPP_INCLUDED
