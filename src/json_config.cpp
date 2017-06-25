#ifndef JSON_CONFIG_WRITTER_CPP
#define JSON_CONFIG_WRITTER_CPP

#define JSON_INPUTS_HEADER "INPUTS"
#define JSON_OUTPUTS_HEADER "OUTPUTS"
#define JSON_CONNECTIONS_HEADER "CONNECTIONS"

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <json/json.h>
#include "config_writer.h"

class JSONWriter : public ConfigWriter {
    public:
        using ConfigWriter::ConfigWriter;

        void load() {
            std::ifstream cfg_file(m_filename);

            Json::Value root;
            cfg_file >> root;
            cfg_file.close();

            //
            // === LOAD INPUTS ===
            //

            m_input_volumes = {};

            for (std::string input : root["INPUTS"].getMemberNames()) {
                std::cout << input+": " << root["INPUTS"][input].asFloat() << std::endl;
                m_input_volumes[input] = root["INPUTS"][input].asFloat() / 100;
            }

            //
            // === LOAD OUTPUTS ===
            //

            m_output_volumes = {};

            for (std::string output : root["OUTPUTS"].getMemberNames()) {
                std::cout << output+": " << root["OUTPUTS"][output].asFloat() << std::endl;
                m_output_volumes[output] = root["OUTPUTS"][output].asFloat() / 100;
            }

            //
            // === LOAD CONNECTIONS ===
            //

            m_connections = {};

            for (std::string output : root["CONNECTIONS"].getMemberNames()) {
                Json::Value connection = root["CONNECTIONS"][output];
                std::vector<std::string> connected;
                for (Json::Value input : connection) {
                    connected.push_back(input.asString());
                    std::cout << output+"->"+input.asString() << std::endl;
                }
                m_connections[output] = connected;
            }


        }

        void save() {
            Json::Value root;
            for (const auto& p : m_input_volumes)
                root["INPUTS"][p.first] = p.second * 100;

            for (const auto& p : m_output_volumes)
                root["OUTPUTS"][p.first] = p.second * 100;

            for (const auto& p : m_connections) {
                for (size_t i=0; i<p.second.size(); i++)
                    root["CONNECTIONS"][p.first][(int)i] = p.second[i];
            }

            std::ofstream cfg_file(m_filename);
            cfg_file << root << '\n';
            /* std::cout << root << std::endl; */
            cfg_file.close();

        }
};

#endif
