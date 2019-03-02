/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#ifndef __SOCKET_HPP_INCLUDED
#define __SOCKET_HPP_INCLUDED

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <memory>
#include <signal.h>
#include <assert.h>

#include "config_parser.hpp"
#include "output.hpp"

class Socket;

class Client
{
    public:
    Client(Socket *server, int descriptor);
    ~Client();
    int get_descriptor() const;
    bool is_connected() const;
    in_addr_t host;
    bool connected = false;

    bool Send(std::string str);
    std::string Recv(size_t len);

    std::queue<std::string> retryqueue;

    private:
    Socket *server;
    int descriptor;
};

class Socket
{
    public:
    enum SelectMode
    {
        SELECT_WRITE,
        SELECT_READ
    };

    Socket(std::shared_ptr<Config> config);
    ~Socket();
    in_addr_t resolve(std::string addr);
    int Bind();
    int Listen();
    Client* HandleNew();
    void Close(Client *c);
    int Select(SelectMode mode, double _timeout = 0);

    std::string addr_to_string(in_addr_t addr);
    in_addr_t string_to_addr(std::string addr);

    std::vector<Client*> clients;

    void set_max_connections_per_ip(size_t val);
    size_t get_max_connections_per_ip() const;

    int get_master_sock_descriptor();

    private:
    int sock_descriptor;
    size_t max_connections;
    size_t max_connections_per_ip;
    size_t throttle_time;
    double default_timeout;
    std::shared_ptr<Config> config;
    std::map<in_addr_t, size_t> throttle;

    int raise_socket_error(int _errno);
};

#endif // __SOCKET_HPP_INCLUDED
