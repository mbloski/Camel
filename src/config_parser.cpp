/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "config_parser.hpp"

Config::Config(std::string master_filename)
{
    int state = parse(master_filename);
    if (state != PARSER_ERROR_OK)
    {
        exit(state);
    }
}

int Config::parse(std::string master_filename)
{
    std::string line;

    /* Push the master file into the queue to begin with.
       Other potential files will be added as the master file is parsed. */
    this->config_files.push(master_filename);

    while (!this->config_files.empty())
    {
        this->current_config_filename = this->config_files.front();

        /* Get current directory */
        size_t pos = this->current_config_filename.find_last_of("/");
        if (pos == std::string::npos)
        {
            this->current_config_filename = this->current_directory + this->current_config_filename;
        }
        else
        {
            std::string curdir = this->current_config_filename.substr(0, pos + 1);
            if (curdir != this->current_directory)
            {
                this->current_directory = this->current_directory + curdir;
            }
            else
            {
                this->current_directory = curdir;
            }

            this->current_config_filename = this->current_directory + this->current_config_filename.substr(pos + 1, -1);
        }

        out::info("Loading '%s'...", out::colorize(this->current_config_filename, out::COLOR_WHITE).c_str());
        this->file_history.push_back(this->current_config_filename);
        this->linecounter = 0;
        std::ifstream current_config_file(this->current_config_filename.c_str());
        this->config_files.pop();
        if (current_config_file)
        {
            while (std::getline(current_config_file, line))
            {
                ++this->linecounter;
                line = line.substr(0, line.find(CHAR_COMMENT));
                if (line.empty())
                {
                    continue;
                }

                std::vector<std::string> kvars = util::tokenize(line, CHAR_KEYWORD);
                if (kvars.size() > 2)
                {
                    /* Two ReservedCharacters found in one line. Raise PARSER CONFUSION. */
                    return raise_parser_error(PARSER_ERROR_CONFUSION);
                }
                else if (line.find(CHAR_KEYWORD) != std::string::npos && kvars.size() < 2)
                {
                    return raise_parser_error(PARSER_ERROR_SYNTAX);
                }

                for (std::string &value : kvars)
                {
                    value = util::trim(value);
                }

                switch(get_keyword(kvars[0]))
                {
                    case KEYWORD_INCLUDE:
                        if (std::find(this->file_history.begin(), this->file_history.end(), kvars[1]) != this->file_history.end())
                        {
                            return raise_parser_error(PARSER_ERROR_RECURSION_TOO_DEEP);
                        }
                        this->config_files.push(kvars[1]);
                        continue;
                    break;
                    case KEYWORD_NONE:
                        if (line.find(CHAR_KEYWORD) != std::string::npos)
                        {
                            return raise_parser_error(PARSER_ERROR_SYNTAX);
                        }
                    break;
                }

                /* We're done parsing keywords. Let's head to the variables. */

                std::vector<std::string> vars = util::tokenize(line, CHAR_ASSIGN);
                if (vars.size() > 2)
                {
                    return raise_parser_error(PARSER_ERROR_CONFUSION);
                }
                else if (vars.size() < 2)
                {
                    return raise_parser_error(PARSER_ERROR_SYNTAX);
                }

                for (std::string &value : vars)
                {
                    value = util::trim(value);
                }

                if (this->variables[vars[0]].empty())
                {
                    this->variables[vars[0]] = vars[1];
#ifdef DEBUG
                    out::debug("[%s] %s = %s", this->current_config_filename.c_str(), vars[0].c_str(), vars[1].c_str());
#endif // DEBUG
                }
                else
                {
                    return raise_parser_error(PARSER_ERROR_VARIABLE_REDEFINITION);
                }
            }
        }
        else
        {
            return raise_parser_error(PARSER_ERROR_READ);
        }
    }

    return PARSER_ERROR_OK;
}

enum Config::ParserErrors Config::raise_parser_error(enum ParserErrors error)
{
    std::string error_message;
    switch (error)
    {
        case PARSER_ERROR_READ:
            error_message = strerror(errno);
        break;
        case PARSER_ERROR_CONFUSION:
            error_message = "Two or more reserved characters found";
        break;
        case PARSER_ERROR_VARIABLE_REDEFINITION:
            error_message = "Variable redefinition";
        break;
        case PARSER_ERROR_RECURSION_TOO_DEEP:
            error_message = "Infinite loop";
        break;
        case PARSER_ERROR_SYNTAX:
            error_message = "Syntax error";
        break;
        case PARSER_ERROR_OK:
            /* Just to silence the compiler. */
        break;
    }

    if (this->linecounter > 0)
    {
        error_message += " on " + out::colorize("line " + std::to_string(this->linecounter), out::COLOR_WHITE);
    }

    out::error("There was an error while reading a config file:");
    out::error("^ \"%s\": %s.", out::colorize(this->current_config_filename, out::COLOR_WHITE).c_str(), error_message.c_str());

    /* I'd rather not want to store values from a faulty config file and keep parsing it.
       Let's abort the process. */
    while (!this->config_files.empty())
    {
        this->config_files.pop();
    }
    this->variables.clear();

    return error;
}

enum Config::ReservedKeywords Config::get_keyword(std::string keyword_str)
{
    if (keyword_str == "INCLUDE")
    {
        return KEYWORD_INCLUDE;
    }

    return KEYWORD_NONE;
}

std::string Config::get_string(std::string var)
{
    if (!this->variables[var].empty())
    {
        return this->variables[var];
    }

    return "";
}

bool Config::get_bool(std::string var)
{
    if (!this->variables[var].empty())
    {
        std::string t = this->variables[var];
        std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        return (t == "yes" || t == "true" || t == "on" || t == "y" || t == "1")? true : false;
    }

    return false;
}
