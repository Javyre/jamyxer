#include "main.h"
#include "package_config.h"
#include "commands.h"
#include "server.h"

#include <iostream>
#include <vector>
#include <utility>
#include <regex>
#include <thread>
#include <chrono>

#include <csignal>

#ifdef WITH_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#else
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif

bool g_interrupt = false;
Server* g_server;

///
/// Main function
///
int main(int argc, char* argv[]) {
    Backend backend(JACK_CLIENT_NAME);

    // Handle interrupt signal
    /* auto int_handler = [&](){ */
    /*     while (!g_interrupt) { */
    /*         std::this_thread::sleep_for(std::chrono::seconds(1)); */
    /*     } */
    /*     std::cout << "Stopping..." << std::endl; */
    /*     backend.stop_recon_loop(); */
    /*     backend.shutdown(); */
    /*     exit(0); */
    /* }; */

    /* std::signal(SIGINT, [](int s){ g_interrupt = true; }); */
    /* std::thread int_handler_thread(int_handler); */

    // Start jack reconnection loop
    backend.start_recon_loop();

    // Start socket listener
    Server server(&backend);
    server.start();
    g_server = &server;

    // Start cmd loop (runs until `stop` command)
    std::thread cmd_thread(cmd_loop, &backend, &server);

    server.m_listener_thread.join();

    std::cout << "Stopping..." << std::endl;
    /* server.stop(); */
    cmd_thread.join();
    backend.stop_recon_loop();
    backend.shutdown();

    return 0;
}


///
/// Command loop
///
void cmd_loop(Backend* backend, Server* server) {
    CommandHandler command_handler(backend);



#ifdef WITH_READLINE
    char * c_cmd_line = (char *)NULL;
    rl_event_hook = []()->int{
        if (g_server->stopped) rl_done = true;
        return 0;
    };
#endif


    for (std::string cmd_line="";;) {
#ifndef WITH_READLINE
        ::fd_set rfds;
        int fd = 0;

        std::cout << "[" PACKAGE_STRING "] >>> " << std::flush;
        int retval;
        do {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);
            struct ::timeval timeout { .tv_sec=1, .tv_usec=0 };
            retval = ::select(fd + 1, &rfds, NULL, NULL, &timeout);

            if (server->stopped) break;
        } while (retval == 0);
        if (retval == -1) std::perror("select()");

        if (!server->stopped) {
            std::getline(std::cin, cmd_line);
        }
#endif
        if (cmd_line == "stop" || cmd_line == "quit")
            break;

        if (server->stopped)
            break;
        if (cmd_line != "") {
            try {
                std::cout << command_handler.run(cmd_line) << std::endl;
            } catch (CommandHandler::CommandHandlerException& e) {
                std::cout << e.what() << std::endl;
            } catch (Settings::SettingsException& e) {
                std::cout << e.what() << std::endl;
            }
        }

#ifdef WITH_READLINE
        if(c_cmd_line) {
            free(c_cmd_line);
            c_cmd_line = (char *)NULL;
        }

        c_cmd_line = readline("[" PACKAGE_STRING "] >>> ");

        if (c_cmd_line && *c_cmd_line)
            add_history(c_cmd_line);
        if (!c_cmd_line)
            break;
        else
            cmd_line = std::string(c_cmd_line);
#endif

    }

#ifdef WITH_READLINE
    if(c_cmd_line)
        free(c_cmd_line);
        c_cmd_line = NULL;
    rl_clear_history();
    clear_history();
#endif

    if (!server->stopped) {
        server->stop();
    }
#undef GET_NEXT_COMMAND_LINE
}

// vim:foldmethod=marker
