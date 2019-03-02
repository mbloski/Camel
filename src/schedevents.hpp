/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __SCHEDEVENTS_HPP_INCLUDED
#define __SCHEDEVENTS_HPP_INCLUDED

#include <list>
#include <chrono>

#include "output.hpp"
#include "client.hpp"

class Server;
class EOClient;

class SchedEvents
{
    public:
    enum EventType
    {
        EVENT_ONEFOLD = 0,
        EVENT_REPEATABLE
    };

    SchedEvents();
    ~SchedEvents();

    void add(EventType type, size_t delay, void (*task_ptr)(Server*, EOClient*, std::string), EOClient* client = nullptr, std::string args = "");
    void cleanup(EOClient *client);
    void Tick(Server *server);

    private:
    struct event
    {
        EventType type = EventType::EVENT_ONEFOLD;
        std::chrono::time_point<std::chrono::system_clock> time;
        size_t delay;
        void *task_ptr = nullptr;
        EOClient *client_ptr = nullptr;
        std::string args = "";
    };

    std::list<event> tasks;

};

#endif // __SCHEDEVENTS_HPP_INCLUDED
