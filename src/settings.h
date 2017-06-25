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
        std::map<std::string, float> m_input_volumes;
        std::map<std::string, float> m_output_volumes;
        std::map<std::string, std::vector<std::string>> m_connections;
        std::string m_filename;

    public:
        class SettingsException : public std::exception {};

        class InputNotFound : public SettingsException {
            const std::string m_input;
            public:
                InputNotFound(const std::string& input) : m_input(input) {}
            const char* what() const throw() {
                const char* out = ("Input not found: `"+m_input+"`").c_str();
                return out;
            }
        };

        class OutputNotFound : public SettingsException {
            const std::string m_output;
            public:
                OutputNotFound(const std::string& output) : m_output(output) {}
            const char* what() const throw() {
                const char* out = ("Output not found: `"+m_output+"`").c_str();
                return out;
            }
        };

        explicit Settings(const std::string filename);
        ~Settings();

        void load();
        void save(); // TODO

        const std::vector<std::string> get_inputs();
        const std::vector<std::string> get_outputs();

        const std::vector<std::string> get_connections(const std::string& output);

        const bool is_input(const std::string& input);
        const bool is_output(const std::string& output);

        void add_input(const std::string& name, float vol=1);
        void add_output(const std::string& name, float vol=1);

        void remove_input(const std::string& input);
        void remove_output(const std::string& output);

        const float get_input_volume(const std::string& input);
        const float get_output_volume(const std::string& output);

        void set_input_volume(const std::string& input, float new_vol);
        void set_output_volume(const std::string& output, float new_vol);

        void connect(const std::string& input, const std::string& output);
        void disconnect(const std::string& input, const std::string& output);


};

#endif
