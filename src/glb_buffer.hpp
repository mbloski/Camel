/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __GLB_BUFFER_HPP_DEFINED
#define __GLB_BUFFER_HPP_DEFINED

#include <ctime>
#include <string>
#include <list>

#include "output.hpp"

class GlbBuffer
{
public:
    GlbBuffer(size_t size);
    ~GlbBuffer();
    void Add(std::string nickname, std::string message);
    std::list<std::pair<time_t, std::pair<std::string, std::string>>> Get() const;
    
private:
    size_t size = 7;
    std::list<std::pair<time_t, std::pair<std::string, std::string>>> buffer;
};

#endif // __GLB_BUFFER_HPP_DEFINED