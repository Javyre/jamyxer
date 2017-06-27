#ifndef BACKEND_H
#define BACKEND_H

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <thread>

#include <jack/jack.h>
#include "settings.h"

class Backend {
    private:
        const std::string m_client_name;

        jack_client_t* m_client = 0;

        std::map<std::string, std::vector<jack_port_t*>> m_input_ports;
        std::map<std::string, std::vector<jack_port_t*>> m_implicit_output_ports;
        std::map<std::string, std::vector<jack_port_t*>> m_explicit_output_ports;

        bool m_try_recon;
        std::thread m_recon_loop;

        int callback(jack_nframes_t nframes);

    public:
        class BackendException : public std::exception {};
        class JackServerIsDown : public BackendException {};

        Settings settings;
        explicit Backend(const std::string client_name);
        ~Backend();
        void setup();
        void start_recon_loop();
        void stop_recon_loop();
        void shutdown();

        void register_port(const std::string name, const bool input);
        void unregister_port(const std::string name, const bool input);
        void rename_port(const std::string old_name, const std::string new_name, const bool input);
};

#endif
