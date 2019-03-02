/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __CONFIG_PARSER_HPP_INCLUDED
#define __CONFIG_PARSER_HPP_INCLUDED

#include <fstream>
#include <errno.h>
#include <cstring>
#include <queue>
#include <map>

#include "output.hpp"

class Config
{
    public:
    Config(std::string master_filename);

    std::string get_string(std::string var);
    bool get_bool(std::string var);

    template<typename T> T get_number(const std::string &var)
    {
        if (this->variables[var].empty())
        {
            return 0;
        }

        std::stringstream s(this->variables[var]);
        T ret;
        return s >> ret? ret : 0;
    }

    private:
    std::string current_config_filename;
    std::queue<std::string> config_files;
    std::vector<std::string> file_history;
    std::string current_directory;
    size_t linecounter;

    std::map<std::string, std::string> variables;

    enum ReservedCharacters
    {
        CHAR_KEYWORD = ':',
        CHAR_ASSIGN = '=',
        CHAR_COMMENT = '#'
    };

    enum ReservedKeywords
    {
        KEYWORD_NONE = -1,
        KEYWORD_INCLUDE
    };

    enum ParserErrors
    {
        PARSER_ERROR_OK = 0,
        PARSER_ERROR_READ = -20,
        PARSER_ERROR_CONFUSION,
        PARSER_ERROR_VARIABLE_REDEFINITION,
        PARSER_ERROR_RECURSION_TOO_DEEP,
        PARSER_ERROR_SYNTAX
    };

    int parse(std::string master_filename);

    enum ParserErrors raise_parser_error(enum ParserErrors error);
    enum ReservedKeywords get_keyword(std::string keyword_str);
};

#endif // __CONFIG_PARSER_HPP_INCLUDED
