#ifndef YAML_CONFIG_WRITTER_CPP
#define YAML_CONFIG_WRITTER_CPP

#define YAML_INPUTS_HEADER "INPUTS"
#define YAML_OUTPUTS_HEADER "OUTPUTS"
#define YAML_CONNECTIONS_HEADER "CONNECTIONS"

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "config_writer.h"

class YAMLWriter : public ConfigWriter {
    public:
        using ConfigWriter::ConfigWriter;

        void load() {
            std::cout << "loading from file: " << m_filename << std::endl;

            YAML::Node config = YAML::LoadFile(m_filename);

            YAML::Node inputs_node = config[YAML_INPUTS_HEADER];
            YAML::Node outputs_node = config[YAML_OUTPUTS_HEADER];
            YAML::Node connections_node = config[YAML_CONNECTIONS_HEADER];

            //
            // === LOAD INPUTS ===
            //

            m_input_volumes = {};
            for (const auto& p : inputs_node)
                m_input_volumes[p.first.as<std::string>()] = p.second.as<float>() / 100;

            std::cout << "Loaded " << m_input_volumes.size() << " inputs:\n";
            for (const auto& k : m_input_volumes)
                std::cout << ". " << k.first << ": " << k.second << '\n';

            //
            // === LOAD OUTPUTS ===
            //

            m_output_volumes = {};
            for (const auto& p : outputs_node)
                m_output_volumes[p.first.as<std::string>()] = p.second.as<float>() / 100;

            std::cout << "Loaded " << m_input_volumes.size() << " outputs:\n";
            for (const auto& k : m_input_volumes)
                std::cout << ". " << k.first << ": " << k.second << '\n';

            //
            // === LOAD CONNECTIONS ===
            //

            m_connections = {};

            for (const auto& connection : connections_node){
                std::string output_name = connection.first.as<std::string>();

                if (m_output_volumes.find(output_name) == m_output_volumes.end()) {
                    std::cerr << "ERROR: Unknown output: " << output_name << std::endl;
                    exit(1);
                }

                std::cout << "connection: " << output_name << std::endl;

                std::vector<std::string> inputs = {};
                for (const auto& input : connection.second) {
                    std::string input_name = input.as<std::string>();

                    if (m_input_volumes.find(input_name) != m_input_volumes.end()) {
                        inputs.push_back(input_name);
                        std::cout << ". " << input_name << std::endl;
                    } else {
                        std::cerr << "ERROR: Unknown input: " << input_name << std::endl;
                        exit(1);
                    }
                }

                m_connections[output_name] = inputs;
            }

        }

        void save() {}
};

#endif
