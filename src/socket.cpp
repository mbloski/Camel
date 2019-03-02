/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "socket.hpp"

Socket::Socket(std::shared_ptr<Config> config)
{
    this->config = config;
    this->sock_descriptor = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    fcntl(this->sock_descriptor, F_SETFL, O_NONBLOCK);

    this->max_connections = this->config->get_number<size_t>("max_connections");
    this->max_connections_per_ip = this->config->get_number<size_t>("max_connections_per_ip");
    this->default_timeout = this->config->get_number<double>("default_socket_timeout");
    this->throttle_time = this->config->get_number<size_t>("throttle_time");

    /* Do some initial checks */
    if (this->max_connections == 0 || this->max_connections_per_ip == 0)
    {
        out::warning("PLEASE NOTE: max_connections or max_connections_per_ip is set to 0");
        out::warning("^ This means the server will refuse *all* incoming connections.");
        out::warning("^ This is probably not what you want. Please correct your configuration.");
    }

    if (this->default_timeout == 0)
    {
        out::warning("default_socket_timeout is set to 0. Defaulting to 0.01.");
        this->default_timeout = 0.01;
    }

    if (this->throttle_time == 0)
    {
        out::warning("throttle_time is set to 0. Defaulting to 15.");
        this->throttle_time = 15;
    }
}

Socket::~Socket()
{
    close(this->sock_descriptor);
}

int Socket::Bind()
{
    int y = 1;
    int res = setsockopt(this->sock_descriptor, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    if (res == -1)
    {
        exit(raise_socket_error(errno));
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    bool errors = false;
    in_addr_t bindhost = inet_addr(this->config->get_string("bind_host").c_str());
    if ((int)bindhost == -1)
    {
        errors = true;
        in_addr_t default_bindhost = INADDR_ANY;
        out::warning("Wrong bind_host specified (%s). Defaulting to %s.", this->config->get_string("bind_host").c_str(), addr_to_string(default_bindhost).c_str());
        addr.sin_addr.s_addr = default_bindhost;
    }
    else
    {
        addr.sin_addr.s_addr = bindhost;
    }

    unsigned short port = this->config->get_number<unsigned short>("port");
    if (port == 0 || this->config->get_number<unsigned short>("port") > 65535)
    {
        errors = true;
        unsigned short default_port = 8078;
        out::warning("Wrong port specified (%d). Defaulting to %d.", this->config->get_number<unsigned short>("port"), default_port);
        addr.sin_port = htons(default_port);
    }
    else
    {
        addr.sin_port = htons(port);
    }

    bind(this->sock_descriptor, (struct sockaddr *)&addr, sizeof(addr));
    if (errno != 0)
    {
        exit(raise_socket_error(errno));
    }

    if (errors)
    {
        out::warning("PLEASE NOTE: The server might work fine, but you have errors in your configuration.");
        out::warning("^ You should really not ignore this.");
    }

    return 0;
}

int Socket::Listen()
{
    int res = listen(this->sock_descriptor, 10);

    if (res == -1)
    {
        exit(raise_socket_error(errno));
    }

    out::info("Ready to accept connections on %s:%d.", this->config->get_string("bind_host").c_str(), this->config->get_number<unsigned short>("port"));
    return res;
}

Client* Socket::HandleNew()
{
    int r = this->Select(Socket::SELECT_READ);

    if (!r)
    {
        return 0;
    }

    struct sockaddr_in new_client;
    socklen_t addr_size = sizeof(new_client);
    int fd = accept(this->sock_descriptor, (struct sockaddr*)&new_client, &addr_size);

    Client *client = nullptr;

    if (fd != -1)
    {
        client = new Client(this, fd);

        struct sockaddr_in n;
        getpeername(fd, (struct sockaddr*)&n, &addr_size);
        in_addr_t host = n.sin_addr.s_addr;

        if (clients.size() >= this->max_connections)
        {
            close(fd);
            out::warning("A connection from %s was closed for exceeding the connection limit.", this->addr_to_string(host).c_str());
            return nullptr;
        }

        size_t ip_counter = 1;
        for (Client *c : this->clients)
        {
            if (c->host == host)
            {
                ++ip_counter;
            }
        }

        if (ip_counter > this->max_connections_per_ip)
        {
            /* Detected more connections from one IP than allowed.
               Let's close the client, throw a warning and forget about it. */
            close(fd);
            out::warning("A connection from %s was closed for exceeding the connection limit for this IP.", this->addr_to_string(host).c_str());
            return nullptr;
        }

        if (std::time(0) - this->throttle[host] >= this->throttle_time)
        {
            this->throttle.erase(host);
        }
        else
        {
            /* Client reconnecting too fast. */
            close(fd);
            out::warning("A connection from %s was closed (throttled).", this->addr_to_string(host).c_str());
            return nullptr;
        }

        client->host = host;
        fcntl(fd, F_SETFL, O_NONBLOCK);

        clients.push_back(client);
        this->throttle[host] = std::time(0);
        out::info("New connection from %s", this->addr_to_string(client->host).c_str());
    }

    return client;
}

int Socket::Select(SelectMode mode, double _timeout)
{
    if (_timeout == 0.0)
    {
        _timeout = this->default_timeout;
    }

    struct timeval timeout;
    timeout.tv_sec = long(_timeout);
    timeout.tv_usec = long((_timeout - double(long(_timeout))) * 1000000);
    fd_set _set;
    FD_ZERO(&_set);
    FD_SET(this->sock_descriptor, &_set);

    fd_set *read_set = mode == SELECT_READ? &_set : NULL;
    fd_set *write_set = mode == SELECT_WRITE? &_set : NULL;

    int res = select(this->sock_descriptor + 1, read_set, write_set, NULL, &timeout);

    if (res <= 0)
    {
        return false;
    }

    return true;
}

in_addr_t Socket::resolve(std::string addr)
{
    hostent *h;
    h = gethostbyname(addr.c_str());

    in_addr_t ret = 0;

    if (h)
    {
        ret = *(in_addr_t *)h->h_addr;
    }

    return ret;
}

std::string Socket::addr_to_string(in_addr_t addr)
{
    struct in_addr tmp;
    tmp.s_addr = addr;
    char *ret = inet_ntoa(tmp);
    std::memset(&tmp, 0, sizeof(tmp));

    return std::string(ret);
}

in_addr_t Socket::string_to_addr(std::string addr)
{
    unsigned char o1, o2, o3, o4;
    sscanf(addr.c_str(), "%hhu.%hhu.%hhu.%hhu", &o1, &o2, &o3, &o4);

    return (o1 << 24) | (o2 << 16) | (o3 << 8) | o4;
}

inline int Socket::raise_socket_error(int _errno)
{
    if (_errno != 0)
    {
        out::error("There was an error with the socket:");
        out::error("^ '%s'", out::colorize(strerror(_errno), out::COLOR_WHITE).c_str());
    }
    return _errno;
}

void Socket::Close(Client *c)
{
    close(c->get_descriptor());
    std::vector<Client*>::iterator pos = std::find(this->clients.begin(), this->clients.end(), c);
    if (pos != this->clients.end())
    {
        this->clients.erase(pos);
    }
}

int Socket::get_master_sock_descriptor()
{
    return this->sock_descriptor;
}

void Socket::set_max_connections_per_ip(size_t val)
{
    this->max_connections_per_ip = val;
    /* We might want to disconnect the connections that reached the newly set limit here. */
}

size_t Socket::get_max_connections_per_ip() const
{
    return this->max_connections_per_ip;
}

Client::Client(Socket *server, int descriptor)
{
    this->descriptor = descriptor;
    this->connected = true;
    this->server = server;
}

Client::~Client()
{

}

int Client::get_descriptor() const
{
    return this->descriptor;
}

bool Client::is_connected() const
{
    return this->connected;
}


bool Client::Send(std::string str)
{
    int bytes_left = str.length();
    int bytes;
    const char *cp = str.c_str();

    while (bytes_left > 0)
    {
        bytes = send(this->get_descriptor(), cp, bytes_left, 0);

        if (bytes <= 0)
        {
            if (errno == 11)
            {
                this->retryqueue.push(cp);
            }
            else
            {
                /* this really needs further handling */
                this->connected = false;
            }

            return false;
        }

        assert(bytes <= bytes_left); // So brutal. TODO: handle this better.

        bytes_left -= bytes;
        cp += bytes;
    }

    return true;
}

std::string Client::Recv(size_t len)
{
    char buf;
    std::string ret = "";

    for (size_t i = 0; i < len; ++i)
    {
        int res = recv(this->get_descriptor(), &buf, 1, 0);
        if (res == 0)
        {
            this->connected = false;
            return ret;
        }
        else if (res == -1)
        {
            return ret;
        }

        ret += buf;
    }

    return ret;
}
