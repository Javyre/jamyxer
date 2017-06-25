#include "commands.h"
#include <utility>
#include <regex>

std::string vol(std::vector<std::string> args, Backend* backend) {
    std::string target_type;
    std::string action;
    std::string target;
    float volume;
    if (args.size() >= 3) {
        target_type = args[0];
        action      = args[1];
        target      = args[2];
    } else
        throw CommandHandler::InvalidNArgs(3, args.size());

    if (action != "get") {
        if (args.size() >= 4)
            volume = std::stof(args[3]);
        else
            throw CommandHandler::InvalidNArgs(4, args.size());
    }

    std::function<void(std::string, float)> set_function;
    std::function<float(std::string)> get_function;

    if (target_type == "input" || target_type == "in") {
        set_function = [&](std::string t, float v){ backend->settings.set_input_volume(t, v); };
        get_function = [&](std::string t){ return backend->settings.get_input_volume(t); };
    } else if (target_type == "output" || target_type == "out") {
        set_function = [&](std::string t, float v){ backend->settings.set_output_volume(t, v); };
        get_function = [&](std::string t){ return backend->settings.get_output_volume(t); };
    } else
        throw CommandHandler::CommandException("Error: unrecognized target type: " + target_type);


    if (action == "set")
        set_function(target, volume/100);
    else if (action == "mod")
        set_function(target, get_function(target) + (volume/100));
    else if (action == "get")
        return std::to_string(get_function(target)*100);
    else
        throw CommandHandler::CommandException("Error: unrecognized action: " + action);

    return std::string("Current volume level for input port `" + target +
        "`: " + std::to_string(get_function(target)*100));

}

std::string connect_command(std::vector<std::string> args, Backend* backend, const bool disconnect=false) {
    if (args.size() != 2)
        throw CommandHandler::InvalidNArgs(4, args.size());

    std::string input  = args[0];
    std::string output = args[1];

    if (!disconnect)
        backend->settings.connect(input, output);
    else
        backend->settings.disconnect(input, output);

    std::string connected_inputs;
    for (std::string i : backend->settings.get_connections(output))
        connected_inputs += "\n- "+i;

    return std::string("Currently connected ports for output: `"+output+"`:"+connected_inputs);
}

std::string dcon(std::vector<std::string> args, Backend* backend) {
    return connect_command(args, backend, true);
}

std::string con(std::vector<std::string> args, Backend* backend) {
    return connect_command(args, backend, false);
}

std::string get(std::vector<std::string> args, Backend* backend) {
    if (args.size() < 1)
        throw CommandHandler::InvalidNArgs(1, args.size());
    std::string target_type = args[0];
    if (target_type == "connections" && args.size() < 2)
        throw CommandHandler::InvalidNArgs(2, args.size());

    std::function<std::vector<std::string>()> get_func;
    if (target_type == "inputs" || target_type == "ins") {
        get_func = [&](){ return backend->settings.get_inputs(); };
    } else if (target_type == "outputs" || target_type == "outs") {
        get_func = [&](){ return backend->settings.get_outputs(); };
    } else if (target_type == "connections" || target_type == "cons") {
        get_func = [&](){ return backend->settings.get_connections(args[1]); };
    } else
        throw CommandHandler::CommandException("Unknown target_type: `"+target_type+"`");

    std::string out = "";
    for (std::string e : get_func()) {
        out += e+"\n";
    }
    out.pop_back();

    return out;
}

std::string add(std::vector<std::string> args, Backend* backend) {
    if (args.size() < 2)
        throw CommandHandler::InvalidNArgs(2, args.size());
    std::string target_type = args[0];
    std::string name = args[1];
    float vol = (args.size() > 3) ? std::stof(args[2]) : 1;

    bool input;
    if (target_type != "input" || target_type != "output")
        input = true;
    else if (target_type != "in" || target_type != "out")
        input = false;
    else
        throw CommandHandler::CommandException("Invalid target_type: `"+target_type+"`");

    backend->register_port(name, input);
    if (input)
        backend->settings.set_input_volume(name, vol);
    else
        backend->settings.set_output_volume(name, vol);

    return "Added...";
}

std::string rem(std::vector<std::string> args, Backend* backend) {
    if (args.size() < 2)
        throw CommandHandler::InvalidNArgs(2, args.size());
    std::string target_type = args[0];
    std::string name = args[1];

    bool input;
    if (target_type != "input" || target_type != "output")
        input = true;
    else if (target_type != "in" || target_type != "out")
        input = false;
    else
        throw CommandHandler::CommandException("Invalid target_type: `"+target_type+"`");

    backend->unregister_port(name, input);

    return "Removed...";
}

std::string ren(std::vector<std::string> args, Backend* backend) {
    if (args.size() < 3)
        throw CommandHandler::InvalidNArgs(3, args.size());
    std::string target_type = args[0];
    std::string old_name = args[1];
    std::string new_name = args[2];

    bool input;
    if (target_type != "input" || target_type != "output")
        input = true;
    else if (target_type != "in" || target_type != "out")
        input = false;
    else
        throw CommandHandler::CommandException("Invalid target_type: `"+target_type+"`");

    backend->rename_port(old_name, new_name, input);

    return "Renamed "+old_name+" -> "+new_name+" ...";
}

#define CMD_ALIAS(o, e) \
std::string o##_##e(std::vector<std::string> args, Backend* backend){ \
    args.insert(args.begin(), std::string( #e )); \
    return o (args, backend); \
}

CMD_ALIAS(add, in);
CMD_ALIAS(add, out);

CMD_ALIAS(rem, in);
CMD_ALIAS(rem, out);

CMD_ALIAS(ren, in);
CMD_ALIAS(ren, out);

CMD_ALIAS(get, ins);
CMD_ALIAS(get, outs);
CMD_ALIAS(get, cons);

CMD_ALIAS(vol, in);
CMD_ALIAS(vol_in, set);
CMD_ALIAS(vol_in, mod);
CMD_ALIAS(vol_in, get);

CMD_ALIAS(vol, out);
CMD_ALIAS(vol_out, set);
CMD_ALIAS(vol_out, mod);
CMD_ALIAS(vol_out, get);

#undef CMD_ALIAS

commands_map_t gen_abrevs(abrevs_list_t abrevs, std::string ww="") {
    commands_map_t commands;
    for (size_t i=0; i<abrevs.size(); i++) {
        auto g = abrevs[i];
        std::vector<std::string> shorts = std::get<1>(g);
        auto func = std::get<2>(g);

        if (std::get<0>(g) == 0) {
            for (std::string w : shorts) {
                std::cout << "alias: " << ww+w << std::endl;
                commands[ww+w] = {0, func};

                abrevs_list_t sub_abrevs;
                for (size_t ii=1; i+ii<abrevs.size(); ii++){
                    if (std::get<0>(abrevs[i+ii]) > 0) {
                        sub_abrevs.push_back(std::make_tuple(
                                    std::get<0>(abrevs[i+ii])-1,
                                    std::get<1>(abrevs[i+ii]),
                                    std::get<2>(abrevs[i+ii])
                                    ));
                    } else break;
                }
                if (!sub_abrevs.empty()) {
                    auto gg = gen_abrevs(sub_abrevs, ww+w);
                    commands.insert(gg.begin(), gg.end());
                }
            }
        }
    }

    return commands;
}

///
/// Constructor: Initialize m_commands map and m_backend
///
CommandHandler::CommandHandler(Backend* backend) : m_backend(backend) {
    typedef std::vector<std::string> shorts;
    shorts input_shorts  = { "input", "in", "i" };
    shorts output_shorts = {"output", "out", "o"};
    shorts set_shorts = {"set", "s"};
    shorts mod_shorts = {"mod", "m"};
    shorts get_shorts = {"get", "g"};

    // Initialize commands and some aliases...
    abrevs_list_t abrevs = {
        {0, {"volume", "vol", "v"}, vol},

        {1, input_shorts, vol_in},
        {2, set_shorts, vol_in_set},
        {2, mod_shorts, vol_in_mod},
        {2, get_shorts, vol_in_get},

        {1, output_shorts, vol_out},
        {2, set_shorts, vol_out_set},
        {2, mod_shorts, vol_out_mod},
        {2, get_shorts, vol_out_get},

        {0, {"connect", "con", "c"}, con},
        {0, {"disconnect", "dconnect", "discon", "dcon", "disc", "dc"}, dcon},

        {0, get_shorts, get},
        {1, input_shorts, get_ins},
        {1, output_shorts, get_outs},
        {1, {"connections", "cons", "c"}, get_cons},

        {0, {"load", "l"},
            [](std::vector<std::string> a, Backend* b){
                b->shutdown();
                b->settings.load();
                return std::string("Loaded!");
            }},
        {0, {"save", "s"},
            [](std::vector<std::string> a, Backend* b){
                b->settings.save();
                return std::string("Saved!");
            }},

        {0, {"add", "a"}, add},
        {1, input_shorts, add_in},
        {1, output_shorts, add_out},

        {0, {"remove", "rem", "rm"}, rem},
        {1, input_shorts, rem_in},
        {1, output_shorts, rem_out},

        {0, {"rename", "ren", "rn"}, ren},
        {1, input_shorts, ren_in},
        {1, output_shorts, ren_out},
    };

    m_commands = gen_abrevs(abrevs);

}

///
/// Parse line to return cmd string and vector of args
///
std::pair<std::string, std::vector<std::string>> CommandHandler::parse_line(std::string cmd_line) {
    std::regex tok_rgx(R"((("([^"\\]|\\"|\\)*")|\S+))");

    if (cmd_line == "") {
        throw EmptyCommand();
    }

    std::sregex_token_iterator iter(cmd_line.begin(), cmd_line.end(), tok_rgx, 0);
    std::sregex_token_iterator rend;

    if (iter == rend) {
        throw EmptyCommand();
    }

    std::string cmd = *(iter++);
    std::vector<std::string> args;

    // Unstring `"` and push_back to args vector
    for (;iter!=rend; iter++) {
        std::string arg = *iter;
        if (arg[0] == '"')
            arg.erase(0, 1);
        if (*(arg.end()-1) == '"')
            arg.pop_back();
        args.push_back(arg);

        std::cout << arg << " | ";
    }
    std::cout << std::endl;

    return {cmd, args};

}

///
/// Parse and run `cmd` line and return output of command
///
std::string CommandHandler::run(std::string cmd_line) {
    auto cmd_and_args = parse_line(cmd_line);
    std::string cmd = cmd_and_args.first;
    std::vector<std::string> args = cmd_and_args.second;

    if (m_commands.find(cmd) != m_commands.end()) {
        if (m_commands[cmd].first == args.size() || m_commands[cmd].first == 0)
            return m_commands[cmd].second(args, m_backend);
        else
            throw InvalidNArgs(m_commands[cmd].first, args.size());
    } else {
        throw CommandNotFound(cmd);
    }
}


