/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __DATABASE_HPP_INCLUDED
#define __DATABASE_HPP_INCLUDED

#include <memory>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

#include "config_parser.hpp"
#include "character.hpp"

class EOClient;
struct Character;

class Database
{
    public:
    Database(std::shared_ptr<Config> config);
    ~Database();

    std::shared_ptr<sql::ResultSet> generic_query(std::string query, bool fatal = false);

    bool AccountExists(std::string account);
    bool CharacterExists(std::string character);
    bool CreateAccount(std::string name, std::string password, std::string realname, std::string location, std::string email, std::string pcname, size_t hdid);
    bool CreateCharacter(std::string account, std::string name, uint16_t gender, uint16_t race, uint16_t hairstyle, uint16_t haircolor);
    bool DeleteCharacter(unsigned long id);
    bool CreateGuild(std::string tag, std::string name);
    bool DeleteGuild(std::string tag);
    bool UpdateCharacter(Character *character);
    bool AddBan(std::string account, size_t hdid, size_t ip, size_t length = 0);
    bool IsBanned(size_t ip);
    bool IsBanned(std::string account);
    bool IsHDIDBanned(size_t hdid);
    long int GetBanLength(std::string account);
    long int GetBanLength(size_t ip);
    long int GetHDIDBanLength(size_t hdid);
    std::shared_ptr<sql::ResultSet> SelectAccount(std::string account);
    bool SetPassword(std::string account, std::string password);
    std::shared_ptr<sql::ResultSet> GetCharacters(std::string account);
    std::shared_ptr<sql::ResultSet> GetGuild(std::string tag);

    private:
    std::shared_ptr<Config> config;
    std::string database_name;

    sql::mysql::MySQL_Driver *driver;
    sql::Connection *connection;

    void InstallDatabase();
    inline void CheckConsistency();
    inline void PrepareStatements();

    int raise_database_error(std::string str);

    bool Use(std::string database);

    sql::PreparedStatement *get_account_statement;
    sql::PreparedStatement *get_character_statement;
    sql::PreparedStatement *create_account_statement;
    sql::PreparedStatement *set_password_statement;
    sql::PreparedStatement *get_characters_statement;
    sql::PreparedStatement *get_guild_statement;
    sql::PreparedStatement *create_character_statement;
    sql::PreparedStatement *delete_character_statement;
    sql::PreparedStatement *create_guild_statement;
    sql::PreparedStatement *delete_guild_statement;
    sql::PreparedStatement *update_character_statement;
    sql::PreparedStatement *add_ban_statement;
    sql::PreparedStatement *get_ip_ban_statement;
    sql::PreparedStatement *get_account_ban_statement;
    sql::PreparedStatement *get_hdid_ban_statement;
    sql::PreparedStatement *delete_ip_ban_statement;
    sql::PreparedStatement *delete_account_ban_statement;
    sql::PreparedStatement *delete_hdid_ban_statement;
};

#endif // __DATABASE_HPP_INCLUDED
