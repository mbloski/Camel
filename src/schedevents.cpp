/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "schedevents.hpp"

SchedEvents::SchedEvents()
{

}

SchedEvents::~SchedEvents()
{

}

void SchedEvents::add(EventType type, size_t delay, void (*task_ptr)(Server*, EOClient*, std::string), EOClient* client, std::string args)
{
    if (client != nullptr && !client->ingame)
    {
#ifdef DEBUG
        out::debug("I have blocked an attempt to register event for client not in game.");
#endif // DEBUG
        return;
    }

    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now() + std::chrono::duration<int, std::ratio<1, 1000>>(delay);
    event new_event;
    new_event.type = type;
    new_event.time = time;
    new_event.delay = delay;
    new_event.task_ptr = (void*)task_ptr;
    new_event.client_ptr = client;
    new_event.args = args;

    this->tasks.push_back(new_event);

#ifdef DEBUG
    std::string strtype;
    switch(new_event.type)
    {
        case EVENT_ONEFOLD:
            strtype = "onefold";
        break;
        case EVENT_REPEATABLE:
            strtype = "repeatable";
        break;
    }

    if (client != nullptr)
    {
        out::debug("Registered %s event %p for client %u. Executing in %u ms.", strtype.c_str(), task_ptr, client->id, delay);
    }
    else
    {
        out::debug("Registered global %s event %p. Executing in %u ms.", strtype.c_str(), task_ptr, delay);
    }
#endif // DEBUG
}

void SchedEvents::cleanup(EOClient *client)
{
    for (std::list<event>::iterator it = this->tasks.begin(); it != this->tasks.end(); )
    {
#ifdef DEBUG
        void *task_ptr = it->task_ptr;
#endif // DEBUG
        if (it->client_ptr != nullptr && it->client_ptr == client)
        {
#ifdef DEBUG
            out::debug("Event %p is obsolete. Cleanup.", task_ptr);
#endif // DEBUG
            it = this->tasks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void SchedEvents::Tick(Server *server)
{
    bool obsolete = false;

    for (std::list<event>::iterator it = this->tasks.begin(); it != this->tasks.end(); )
    {
        if (std::chrono::system_clock::now() >= it->time)
        {
            void *task_ptr = it->task_ptr;

            void (*f)(Server*, EOClient*, std::string) = (void(*)(Server*, EOClient*, std::string))task_ptr;
            if (it->client_ptr == nullptr || (it->client_ptr)->ingame)
            {
#ifdef DEBUG
                out::debug("Executing event %p.", task_ptr);
#endif // DEBUG
                (*f)(server, (EOClient*)it->client_ptr, it->args);
            }
            else
            {
                obsolete = true;
#ifdef DEBUG
                out::debug("Event %p is obsolete and had to be aborted.", task_ptr);
#endif // DEBUG
            }

            if (!obsolete && it->type == SchedEvents::EventType::EVENT_REPEATABLE)
            {
                this->add(SchedEvents::EventType::EVENT_REPEATABLE, it->delay, f, it->client_ptr, it->args);
            }

            it = this->tasks.erase(it);
        }
        else
        {
            ++it;
        }
    }
}
