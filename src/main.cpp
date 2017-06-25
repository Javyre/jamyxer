#include "main.h"
#include "package_config.h"
#include "commands.h"

#include <iostream>
#include <vector>
#include <utility>
#include <regex>
#include <thread>
#include <chrono>

#include <csignal>

#include <readline/readline.h>
#include <readline/history.h>

bool g_interrupt = false;

///
/// Main function
///
int main(int argc, char* argv[]) {
    std::cout << "Hello, world!" << std::endl;

    Backend backend(JACK_CLIENT_NAME);

    std::cout << "\n--------------\n";

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

    // Start cmd loop (runs until `stop` command)
    cmd_loop(&backend);

    std::cout << "Stopping..." << std::endl;
    backend.stop_recon_loop();
    backend.shutdown();

    return 0;
}


///
/// Command loop
///
void cmd_loop(Backend* backend) {
    CommandHandler command_handler(backend);

    /* using_history(); */
    char * c_cmd_line = (char *)NULL;


#define GET_NEXT_COMMAND_LINE() do { \
    std::cout << "[" PACKAGE_STRING "] >>> "; \
    std::getline(std::cin, cmd_line); continue; \
} while (0);

#define GET_NEXT_COMMAND_LINE() do { \
    if(c_cmd_line) { \
        free(c_cmd_line); \
        c_cmd_line = (char *)NULL; \
    } \
    \
    c_cmd_line = readline("[" PACKAGE_STRING "] >>> "); \
    \
    if (!c_cmd_line) \
        break; \
    if (c_cmd_line && *c_cmd_line) \
        add_history(c_cmd_line); \
    else \
        cmd_line = std::string(c_cmd_line); \
} while(0);

    for (std::string cmd_line=""; cmd_line != "stop" && cmd_line != "quit";) {
        if (cmd_line != "") {
            try {
                std::cout << command_handler.run(cmd_line) << std::endl;
            } catch (CommandHandler::CommandHandlerException& e) {
                std::cout << e.what() << std::endl;
            } catch (Settings::SettingsException& e) {
                std::cout << e.what() << std::endl;
            }
        }

        GET_NEXT_COMMAND_LINE();
    }

if(c_cmd_line)
    free(c_cmd_line);
    c_cmd_line = NULL;
rl_clear_history();
clear_history();

#undef GET_NEXT_COMMAND_LINE
}

// vim:foldmethod=marker
