/* Camel. The Endless Online game server.
 * Copyright (C) 2014, Michael Bloski <michael@blo.ski>
 */

#include "config_parser.hpp"
#include "output.hpp"
#include "util.hpp"
#include "server.hpp"

class SignalException : public std::exception
{
    public:
    SignalException(int sig) : signal(sig) {}
    int signal;
};

int main()
{
#ifdef BIG_STARTUP_LOGO
    /* I don't like this kind of things... Why not make it optional, though. */
    out::gen("  ____                     _ ");
    out::gen(" / ___|__ _ _ __ ___   ___| |");
    out::gen("| |   / _` | '_ ` _ \\ / _ \\ |");
    out::gen("| |__| (_| | | | | | |  __/ |");
    out::gen(" \\____\\__,_|_| |_| |_|\\___|_|");
    out::gen("");
#endif // BIG_STARTUP_LOGO
    out::gen("Camel. The Endless Online game server.");
    out::gen("Copyright (C) 2014, Michael Bloski <michael@blo.ski>");
#ifdef DEBUG
    out::warning("Warning: This is a debug build.");
#endif // DEBUG

    std::shared_ptr<Config> config(new Config("config/master.conf"));

    out::colorful = !config->get_bool("disable_colors");

    Server *gameserver = new Server(config);

    struct sigaction interrupt_handler;
    interrupt_handler.sa_handler = [](int sig){throw SignalException(sig);};
    sigemptyset(&interrupt_handler.sa_mask);
    interrupt_handler.sa_flags = 0;
    sigaction(SIGINT, &interrupt_handler, NULL);
    sigaction(SIGTERM, &interrupt_handler, NULL);

    try
    {
        gameserver->run();
    }
    catch (const SignalException &e)
    {
#ifdef DEBUG
        out::warning("Caught signal %d: %s", e.signal, strsignal(e.signal));
        gameserver->WorldAnnounce("[debug] Caught signal " + std::to_string(e.signal) + ": " + strsignal(e.signal));
#endif // DEBUG
        gameserver->Shutdown();
    }

    delete gameserver;
    return 0;
}
