#ifndef COMMANDS_H
#define COMMANDS_H

#include "backend.h"

#include <string>
#include <vector>
#include <map>

#include <exception>

typedef std::string (* commandref_t)(std::vector<std::string>, Backend*);
typedef std::pair<unsigned int, commandref_t> command_t;
typedef std::map<std::string, command_t> commands_map_t;
typedef std::vector<std::tuple<unsigned int, std::vector<std::string>, commandref_t>> abrevs_list_t;

class CommandHandler {
    private:
        std::map<std::string, command_t> m_commands;
        Backend* m_backend;
        std::pair<std::string, std::vector<std::string>> parse_line(std::string);

    public:
        class CommandHandlerException : public std::exception {};

        class EmptyCommand : public CommandHandlerException {
            const char * what() const throw() {
                return "Empty command line value!";
            }
        };

        class CommandNotFound: public CommandHandlerException {
            private:
                const std::string m_cmd;
            public:
                CommandNotFound(const std::string cmd) : m_cmd(cmd) {}

            const char * what() const throw() {
                const char * out = std::string("Command not found: `" + m_cmd + "`").c_str();
                return out;
            }
        };

        class InvalidNArgs : public CommandHandlerException {
            private:
                const unsigned int m_nargs;
                const unsigned int m_expected;
            public:
                InvalidNArgs(const unsigned int expected, const unsigned int nargs) : m_nargs(nargs), m_expected(expected) {}

            const char * what() const throw() {
                const char * out = std::string("Invalid number of arguments: (expected: "
                        +std::to_string(m_expected)+" received: "+std::to_string(m_nargs)+")").c_str();
                return out;
            }
        };

        class CommandException : public CommandHandlerException {
            private:
                const std::string m_msg;
            public:
                CommandException(const std::string msg) : m_msg(msg) {}

            const char * what() const throw() {
                const char * out = m_msg.c_str();
                return out;
            }
        };

        CommandHandler(Backend* backend);
        std::string run(std::string cmd);

};

#endif
