/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "glb_buffer.hpp"

GlbBuffer::GlbBuffer(size_t size)
{
    this->size = size;
}

void GlbBuffer::Add(std::string nickname, std::string message)
{
    if (this->size == 0)
    {
        return;
    }
    
    if (this->buffer.size() >= this->size)
    {
        this->buffer.pop_front();
    }
    
    this->buffer.push_back(std::make_pair(std::time(0), std::make_pair(nickname, message)));
}

std::list<std::pair<time_t, std::pair<std::string, std::string>>> GlbBuffer::Get() const
{
    return this->buffer;
}

GlbBuffer::~GlbBuffer()
{

}