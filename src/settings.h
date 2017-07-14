#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>
#include <map>
#include <exception>

#include "package_config.h"

#include "json_config.cpp"
#define SETTINGS_BACKEND JSONWriter



/* #define CONFIG_PATH "res/config.yaml" */
#define CONFIG_PATH "res/config.json"


#define LEFT_SUFFIX " L"
#define RIGHT_SUFFIX " R"

#define OUT_SUFFIX " Out"

#define JACK_CLIENT_NAME "jamyxer"

class Settings{
    private:
        std::string m_monitor_channel;
        bool m_monitoring_input;

        std::map<std::string, std::string> m_input_aliases;
        std::map<std::string, std::string> m_output_aliases;

        std::map<std::string, float> m_input_volumes;
        std::map<std::string, float> m_output_volumes;
        std::map<std::string, std::vector<std::string>> m_connections;
        std::string m_filename;

        std::map<std::string, std::vector<int>> m_input_volume_listeners;
        std::map<std::string, std::vector<int>> m_output_volume_listeners;

    public:
        class SettingsException : public std::exception {
            public:
                std::string os;
            const char* what() const throw() {
                return os.c_str();
            }
        };

        class InputNotFound : public SettingsException {
            const std::string m_input;
            public:
                InputNotFound(const std::string& input) : m_input(input) {
                    os = "Input not found: `"+m_input+"`";
                }
        };

        class OutputNotFound : public SettingsException {
            const std::string m_output;
            public:
                OutputNotFound(const std::string& output) : m_output(output) {
                    os = "Output not found: `"+m_output+"`";
                }
        };

        explicit Settings(const std::string filename);
        ~Settings();

        void load();
        void save(); // TODO

        // === name aliases ===
        void gen_aliases();

        const bool is_input_alias(const std::string& input);
        const bool is_output_alias(const std::string& output);

        const std::string get_input_name(const std::string& input);
        const std::string get_output_name(const std::string& output);

        const std::vector<std::string> get_input_aliases();
        const std::vector<std::string> get_output_aliases();

        // === get ===
        const std::string get_monitor();
        const bool monitoring_input();
        const bool monitoring_output();

        const std::vector<std::string> get_inputs();
        const std::vector<std::string> get_outputs();

        const std::vector<std::string> get_connections(const std::string& output);

        const bool is_input(const std::string& input, const bool check_alias=false);
        const bool is_output(const std::string& output, const bool check_alias=false);

        const float get_input_volume(const std::string& input);
        const float get_output_volume(const std::string& output);

        const bool is_connected(const std::string& input, const std::string& output);

        // === set ===
        void monitor_input(const std::string input);
        void monitor_output(const std::string output);

        void add_input(const std::string& name, float vol=1);
        void add_output(const std::string& name, float vol=1);

        void remove_input(const std::string& input);
        void remove_output(const std::string& output);

        void set_input_volume(const std::string& input, float new_vol);
        void set_output_volume(const std::string& output, float new_vol);

        void connect(const std::string& input, const std::string& output);
        void disconnect(const std::string& input, const std::string& output);

        // === misc ===
        void add_input_volume_listener(const std::string& input, const int file_descriptor);
        void add_output_volume_listener(const std::string& output, const int file_descriptor);
};

#endif
