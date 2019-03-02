/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "database.hpp"

Database::Database(std::shared_ptr<Config> config)
{
    this->config = config;
    std::string host = config->get_string("database_host");
    unsigned short port = config->get_number<unsigned short>("database_port");
    this->database_name = config->get_string("database_name");

    if (host.empty())
    {
        out::warning("database_host is not set. Defaulting to '127.0.0.1'.");
        host = "127.0.0.1";
    }

    if (port == 0)
    {
        out::warning("database_port is set to 0. Defaulting to 3306.");
        port = 3306;
    }

    if (this->database_name.empty())
    {
        out::warning("database_name is not set. Defaulting to 'endlessonline'.");
        this->database_name = "endlessonline";
    }

    std::string hostport = host + ":" + std::to_string(port);
    std::pair<std::string, std::string> credentials = std::make_pair(config->get_string("database_username"), config->get_string("database_password"));
    try
    {
        bool reconnect = true;
        this->driver = sql::mysql::get_mysql_driver_instance();
        this->connection = this->driver->connect(hostport, credentials.first, credentials.second);
        this->connection->setClientOption("OPT_RECONNECT", &reconnect);
        out::info("Database connection established: %s.", hostport.c_str());
    }
    catch (sql::SQLException &e)
    {
        exit(raise_database_error(e.what()));
    }

    if (!this->Use(this->database_name))
    {
        std::string message = "Couldn't access the database '" + this->database_name + "'";
        if (!this->config->get_string("database_install_file").empty())
        {
            out::warning("%s. Attempting to install it.", message.c_str());
            this->InstallDatabase();
        }
        else
        {
            out::error("%s and database_install_file is not set. Please specify a file to install the database from or install it manually and try again.", message.c_str());
            exit(0);
        }
    }
    else
    {
        this->CheckConsistency();
    }

    this->PrepareStatements();
}

Database::~Database()
{
    delete this->get_account_statement;
    delete this->get_character_statement;
    delete this->create_account_statement;
    delete this->set_password_statement;
    delete this->get_characters_statement;
    delete this->get_guild_statement;
    delete this->create_character_statement;
    delete this->delete_character_statement;
    delete this->create_guild_statement;
    delete this->delete_guild_statement;
    delete this->update_character_statement;
    delete this->add_ban_statement;
    delete this->get_ip_ban_statement;
    delete this->get_account_ban_statement;
    delete this->get_hdid_ban_statement;
    delete this->delete_ip_ban_statement;
    delete this->delete_account_ban_statement;
    delete this->delete_hdid_ban_statement;
    delete this->connection;
}

void Database::InstallDatabase()
{
    std::string filename = this->config->get_string("database_install_file");
    std::ifstream file(filename);
    if (errno != 0)
    {
        out::error("%s - %s", filename.c_str(), strerror(errno));
        exit(0);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    this->generic_query("CREATE DATABASE IF NOT EXISTS " + this->database_name);
    if (!this->Use(this->database_name))
    {
        out::error("Failed to create the database '%s'.", this->database_name.c_str());
        exit(0);
    }

    std::vector<std::string> queries = util::tokenize(buffer.str(), ';');
    for (std::string query : queries)
    {
        query = util::trim(query);
        if (query.empty())
        {
            continue;
        }

        this->generic_query(query, true);
    }

    this->CheckConsistency();
    out::info("The database '%s' has been installed and is ready for use.", this->database_name.c_str());
}

std::shared_ptr<sql::ResultSet> Database::generic_query(std::string query, bool fatal)
{
    sql::Statement *statement;
    statement = this->connection->createStatement();
    std::shared_ptr<sql::ResultSet> resset;

    try
    {
#ifdef DEBUG
        out::debug("%s", query.c_str());
#endif
        statement->execute(query);
        resset = std::shared_ptr<sql::ResultSet>(statement->getResultSet());
    }
    catch (sql::SQLException &e)
    {
        if (fatal)
        {
            exit(raise_database_error(e.what()));
        }

        return nullptr;
    }

    delete statement;
    return resset;
}

bool Database::Use(std::string database)
{
    sql::Statement *statement;
    statement = this->connection->createStatement();

    try
    {
        statement->execute("USE " + database);
    }
    catch (sql::SQLException &e)
    {
        return false;
    }

    delete statement;
    return true;
}

inline void Database::CheckConsistency()
{
    if (this->generic_query("SHOW TABLES LIKE 'accounts'")->rowsCount() == 1 && this->generic_query("SHOW TABLES LIKE 'characters'")->rowsCount() == 1 && this->generic_query("SHOW TABLES LIKE 'bans'")->rowsCount() == 1)
    {
        std::vector<std::pair<std::string, std::string>> fields;
        fields.push_back(std::make_pair("accounts", "id"));
        fields.push_back(std::make_pair("accounts", "name"));
        fields.push_back(std::make_pair("accounts", "password"));
        fields.push_back(std::make_pair("accounts", "salt"));
        fields.push_back(std::make_pair("accounts", "realname"));
        fields.push_back(std::make_pair("accounts", "location"));
        fields.push_back(std::make_pair("accounts", "email"));
        fields.push_back(std::make_pair("accounts", "pcname"));
        fields.push_back(std::make_pair("accounts", "hdid"));

        fields.push_back(std::make_pair("characters", "id"));
        fields.push_back(std::make_pair("characters", "account"));
        fields.push_back(std::make_pair("characters", "name"));
        fields.push_back(std::make_pair("characters", "sex"));
        fields.push_back(std::make_pair("characters", "race"));
        fields.push_back(std::make_pair("characters", "hairstyle"));
        fields.push_back(std::make_pair("characters", "haircolor"));
        fields.push_back(std::make_pair("characters", "home"));
        fields.push_back(std::make_pair("characters", "partner"));
        fields.push_back(std::make_pair("characters", "title"));
        fields.push_back(std::make_pair("characters", "exp"));
        fields.push_back(std::make_pair("characters", "level"));
        fields.push_back(std::make_pair("characters", "admin"));
        fields.push_back(std::make_pair("characters", "map"));
        fields.push_back(std::make_pair("characters", "x"));
        fields.push_back(std::make_pair("characters", "y"));
        fields.push_back(std::make_pair("characters", "direction"));
        fields.push_back(std::make_pair("characters", "hp"));
        fields.push_back(std::make_pair("characters", "tp"));
        fields.push_back(std::make_pair("characters", "class"));
        fields.push_back(std::make_pair("characters", "str"));
        fields.push_back(std::make_pair("characters", "int"));
        fields.push_back(std::make_pair("characters", "wis"));
        fields.push_back(std::make_pair("characters", "agi"));
        fields.push_back(std::make_pair("characters", "con"));
        fields.push_back(std::make_pair("characters", "cha"));
        fields.push_back(std::make_pair("characters", "karma"));
        fields.push_back(std::make_pair("characters", "usage"));
        fields.push_back(std::make_pair("characters", "arena_wins"));
        fields.push_back(std::make_pair("characters", "arena_kills"));
        fields.push_back(std::make_pair("characters", "arena_deaths"));
        fields.push_back(std::make_pair("characters", "sitting"));
        fields.push_back(std::make_pair("characters", "hidden"));
        fields.push_back(std::make_pair("characters", "guild"));
        fields.push_back(std::make_pair("characters", "guildrank"));
        fields.push_back(std::make_pair("characters", "guildrank_str"));
        fields.push_back(std::make_pair("characters", "inventory"));
        fields.push_back(std::make_pair("characters", "paperdoll"));

        fields.push_back(std::make_pair("guilds", "id"));
        fields.push_back(std::make_pair("guilds", "tag"));
        fields.push_back(std::make_pair("guilds", "name"));
        fields.push_back(std::make_pair("guilds", "description"));
        fields.push_back(std::make_pair("guilds", "ranks"));
        fields.push_back(std::make_pair("guilds", "wealth"));

        fields.push_back(std::make_pair("board_posts", "id"));
        fields.push_back(std::make_pair("board_posts", "board"));
        fields.push_back(std::make_pair("board_posts", "name"));
        fields.push_back(std::make_pair("board_posts", "title"));
        fields.push_back(std::make_pair("board_posts", "post"));

        fields.push_back(std::make_pair("bans", "id"));
        fields.push_back(std::make_pair("bans", "ip"));
        fields.push_back(std::make_pair("bans", "hdid"));
        fields.push_back(std::make_pair("bans", "account"));
        fields.push_back(std::make_pair("bans", "expires"));

        for (std::pair<std::string, std::string> f : fields)
        {
            std::shared_ptr<sql::ResultSet> res = this->generic_query("SHOW COLUMNS FROM " + f.first + " LIKE '" + f.second + "'", true);
            if (res->rowsCount() == 0)
            {
                exit(raise_database_error("The database is inconsistent: column \"" + f.second + "\" in table \"" + f.first + "\" does not exist."));
            }
        }

        return;
    }
    else
    {
        exit(raise_database_error("The database is inconsistent. Please install the missing tables."));
    }
}

inline void Database::PrepareStatements()
{
    try
    {
        this->get_account_statement = this->connection->prepareStatement("SELECT * FROM accounts WHERE name = ?");
        this->get_character_statement = this->connection->prepareStatement("SELECT * FROM characters WHERE name = ?");
        this->create_account_statement = this->connection->prepareStatement("INSERT INTO accounts(name, password, salt, realname, location, email, pcname, hdid) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
        this->set_password_statement = this->connection->prepareStatement("UPDATE accounts SET password = ?, salt = ? WHERE name = ?");
        this->get_characters_statement = this->connection->prepareStatement("SELECT * FROM characters WHERE account = ? ORDER BY level DESC, id ASC");
        this->get_guild_statement = this->connection->prepareStatement("SELECT * FROM guilds WHERE tag = ?");
        this->create_character_statement = this->connection->prepareStatement("INSERT INTO characters(account, name, sex, race, hairstyle, haircolor, inventory, paperdoll) VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
        this->delete_character_statement = this->connection->prepareStatement("DELETE FROM characters WHERE id = ?");
        this->create_guild_statement = this->connection->prepareStatement("INSERT INTO guilds(tag, name, wealth) VALUES(?, ?, ?)");
        this->delete_guild_statement = this->connection->prepareStatement("DELETE FROM guilds WHERE tag = ?");
        this->update_character_statement = this->connection->prepareStatement("UPDATE characters SET name = ?, sex = ?, race = ?, hairstyle = ?, haircolor = ?, home = ?, partner = ?, title = ?, exp = ?, level = ?, admin = ?, map = ?, x = ?, y = ?, direction = ?, hp = ?, tp = ?, class = ?, str = ?, `int` = ?, wis = ?, agi = ?, con = ?, cha = ?, karma = ?, `usage` = ?, arena_wins = ?, arena_kills = ?, arena_deaths = ?, sitting = ?, hidden = ?, guild = ?, guildrank = ?, guildrank_str = ?, inventory = ?, paperdoll = ? WHERE id = ?");
        this->add_ban_statement = this->connection->prepareStatement("INSERT INTO bans(ip, hdid, account, expires) VALUES(?, ?, ?, ?)");
        this->get_ip_ban_statement = this->connection->prepareStatement("SELECT * FROM bans WHERE ip = ?");
        this->get_account_ban_statement = this->connection->prepareStatement("SELECT * FROM bans WHERE account = ?");
        this->get_hdid_ban_statement = this->connection->prepareStatement("SELECT * FROM bans WHERE hdid = ?");
        this->delete_ip_ban_statement = this->connection->prepareStatement("DELETE FROM bans WHERE ip = ?");
        this->delete_account_ban_statement = this->connection->prepareStatement("DELETE FROM bans WHERE account = ?");
        this->delete_hdid_ban_statement = this->connection->prepareStatement("DELETE FROM bans WHERE hdid = ?");
    }
    catch (sql::SQLException &e)
    {
        exit(raise_database_error(e.what()));
    }
}

bool Database::AccountExists(std::string account)
{
    sql::PreparedStatement *statement = this->get_account_statement;
    statement->setString(1, util::strtolower(account));

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    if (result->rowsCount() == 1)
    {
        return true;
    }

    return false;
}

bool Database::CharacterExists(std::string character)
{
    sql::PreparedStatement *statement = this->get_character_statement;
    statement->setString(1, util::strtolower(character));

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    if (result->rowsCount() == 1)
    {
        return true;
    }

    return false;
}

bool Database::CreateAccount(std::string name, std::string password, std::string realname, std::string location, std::string email, std::string pcname, size_t hdid)
{
    std::string salt = util::rand_string(12);
    std::string password_hash = util::sha256(salt + password + salt + salt);
    sql::PreparedStatement *statement = this->create_account_statement;
    statement->setString(1, util::strtolower(name));
    statement->setString(2, password_hash);
    statement->setString(3, salt);
    statement->setString(4, realname);
    statement->setString(5, location);
    statement->setString(6, email);
    statement->setString(7, pcname);
    statement->setUInt(8, hdid);

    bool ret = statement->execute();

    return ret;
}

bool Database::CreateCharacter(std::string account, std::string name, uint16_t gender, uint16_t race, uint16_t hairstyle, uint16_t haircolor)
{
    sql::PreparedStatement *statement = this->create_character_statement;
    statement->setString(1, util::strtolower(account));
    statement->setString(2, util::strtolower(name));
    statement->setUInt(3, gender);
    statement->setUInt(4, race);
    statement->setUInt(5, hairstyle);
    statement->setUInt(6, haircolor);
    statement->setString(7, "1,0");
    statement->setString(8, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0");

    bool ret = statement->execute();

    return ret;
}

bool Database::DeleteCharacter(unsigned long id)
{
    sql::PreparedStatement *statement = this->delete_character_statement;
    statement->setUInt(1, id);

    bool ret = statement->execute();

    return ret;
}

bool Database::CreateGuild(std::string tag, std::string name)
{
    sql::PreparedStatement *statement = this->create_guild_statement;
    statement->setString(1, util::strtoupper(tag).substr(0, 3));
    statement->setString(2, util::strtolower(name));
    statement->setString(3, "1:leader,2:recruiter,3:,4:,5:,6:,7:,8:,9:");

    bool ret = statement->execute();

    return ret;
}

bool Database::DeleteGuild(std::string tag)
{
    sql::PreparedStatement *statement = this->delete_guild_statement;
    statement->setString(1, util::strtoupper(tag).substr(0, 3));

    bool ret = statement->execute();

    return ret;
}

bool Database::UpdateCharacter(Character *character)
{
    sql::PreparedStatement *statement = this->update_character_statement;
    statement->setString(1, character->name);
    statement->setUInt(2, character->gender);
    statement->setUInt(3, character->race);
    statement->setUInt(4, character->hairstyle);
    statement->setUInt(5, character->haircolor);
    statement->setString(6, character->home);
    statement->setString(7, character->partner);
    statement->setString(8, character->title);
    statement->setUInt(9, character->exp);
    statement->setUInt(10, character->level);
    statement->setUInt(11, character->admin);
    statement->setUInt(12, character->position.map);
    statement->setUInt(13, character->position.x);
    statement->setUInt(14, character->position.y);
    statement->setUInt(15, character->position.direction);
    statement->setUInt(16, character->hp);
    statement->setUInt(17, character->tp);
    statement->setUInt(18, character->class_id);
    statement->setUInt(19, character->stats.str);
    statement->setUInt(20, character->stats.intelligence);
    statement->setUInt(21, character->stats.wis);
    statement->setUInt(22, character->stats.agi);
    statement->setUInt(23, character->stats.con);
    statement->setUInt(24, character->stats.cha);
    statement->setUInt(25, character->karma);
    statement->setUInt(26, character->usage + ((std::time(0) - character->online_since) / 60));
    statement->setUInt(27, character->arena_wins);
    statement->setUInt(28, character->arena_kills);
    statement->setUInt(29, character->arena_deaths);
    statement->setUInt(30, character->sitting);
    statement->setUInt(31, character->hidden);
    statement->setString(32, character->guild);
    statement->setUInt(33, character->guild_rank);
    statement->setString(34, character->guild_rank_str);
    statement->setString(35, character->inventory->get_serialized());
    statement->setString(36, character->paperdoll->get_serialized());
    statement->setUInt(37, character->id);

    bool ret = statement->execute();

    return ret;
}

std::shared_ptr<sql::ResultSet> Database::SelectAccount(std::string account)
{
    sql::PreparedStatement *statement = this->get_account_statement;
    statement->setString(1, util::strtolower(account));
    std::shared_ptr<sql::ResultSet> ret = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    return ret;
}

bool Database::SetPassword(std::string account, std::string password)
{
    std::string salt = util::rand_string(12);
    std::string password_hash = util::sha256(salt + password + salt + salt);
    sql::PreparedStatement *statement = this->set_password_statement;
    statement->setString(1, password_hash);
    statement->setString(2, salt);
    statement->setString(3, util::strtolower(account));

    bool ret = statement->execute();

    return ret;
}

std::shared_ptr<sql::ResultSet> Database::GetCharacters(std::string account)
{
    sql::PreparedStatement *statement = this->get_characters_statement;
    statement->setString(1, util::strtolower(account));
    std::shared_ptr<sql::ResultSet> ret = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    return ret;
}

std::shared_ptr<sql::ResultSet> Database::GetGuild(std::string tag)
{
    sql::PreparedStatement *statement = this->get_guild_statement;
    statement->setString(1, util::strtoupper(tag).substr(0, 3));
    std::shared_ptr<sql::ResultSet> ret = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    return ret;
}

bool Database::AddBan(std::string account, size_t hdid, size_t ip, size_t length)
{
    sql::PreparedStatement *statement = this->add_ban_statement;
    statement->setUInt(1, ip);
    statement->setUInt(2, hdid);
    statement->setString(3, account);
    statement->setUInt(4, length == 0? 0 : std::time(0) + length * 60);

    bool ret = statement->execute();

    return ret;
}

bool Database::IsBanned(size_t ip)
{
    sql::PreparedStatement *statement = this->get_ip_ban_statement;
    statement->setUInt(1, ip);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") != 0 && result->getUInt("expires") - std::time(0) <= 0)
        {
            sql::PreparedStatement *statement = this->delete_ip_ban_statement;
            statement->setUInt(1, ip);
            statement->execute();
            break;
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool Database::IsBanned(std::string account)
{
    sql::PreparedStatement *statement = this->get_account_ban_statement;
    statement->setString(1, account);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") != 0 && result->getUInt("expires") - std::time(0) <= 0)
        {
            sql::PreparedStatement *statement = this->delete_account_ban_statement;
            statement->setString(1, account);
            statement->execute();
            break;
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool Database::IsHDIDBanned(size_t hdid)
{
    sql::PreparedStatement *statement = this->get_hdid_ban_statement;
    statement->setUInt(1, hdid);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") != 0 && result->getUInt("expires") - std::time(0) <= 0)
        {
            sql::PreparedStatement *statement = this->delete_hdid_ban_statement;
            statement->setUInt(1, hdid);
            statement->execute();
            break;
        }
        else
        {
            return true;
        }
    }

    return false;
}

long int Database::GetBanLength(std::string account)
{
    long int ret = 0;
    sql::PreparedStatement *statement = this->get_account_ban_statement;
    statement->setString(1, account);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") > 0)
        {
            ret = result->getUInt("expires") - std::time(0);
        }
    }

    return ret;
}

long int Database::GetBanLength(size_t ip)
{
    long int ret = 0;
    sql::PreparedStatement *statement = this->get_ip_ban_statement;
    statement->setUInt(1, ip);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") > 0)
        {
            ret = result->getUInt("expires") - std::time(0);
        }
    }

    return ret;
}

long int Database::GetHDIDBanLength(size_t hdid)
{
    long int ret = 0;
    sql::PreparedStatement *statement = this->get_hdid_ban_statement;
    statement->setUInt(1, hdid);

    std::shared_ptr<sql::ResultSet> result = std::shared_ptr<sql::ResultSet>(statement->executeQuery());

    while (result->next())
    {
        if (result->getUInt("expires") > 0)
        {
            ret = result->getUInt("expires") - std::time(0);
        }
    }

    return ret;
}

int Database::raise_database_error(std::string str)
{
    out::error("There was an error with the database:");
    out::error("^ %s", out::colorize(str, out::COLOR_WHITE).c_str());
    return 0xDB;
}
