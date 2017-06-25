#ifndef CONFIG_WRITER_H
#define CONFIG_WRITER_H

#include <string>
#include <map>
#include <vector>

class ConfigWriter{
    public:
        std::string m_filename;

        explicit ConfigWriter(const std::string filename) : m_filename(filename) { }
        std::map<std::string, float> m_input_volumes;
        std::map<std::string, float> m_output_volumes;
        std::map<std::string, std::vector<std::string>> m_connections;
        virtual void load() = 0;
        virtual void save() = 0;
};

#endif
