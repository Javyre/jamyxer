#include "settings.h"

#include <iostream>
#include <yaml-cpp/yaml.h>

///
/// Constructor:
///     @param filename the path to the config file
///
Settings::Settings(const std::string filename): m_filename(filename) { }

///
/// Load settings from file (filename set in constructor)
///
void Settings::load() {
    SETTINGS_BACKEND backend(m_filename);
    backend.load();
    m_input_volumes  = backend.m_input_volumes;
    m_output_volumes = backend.m_output_volumes;
    m_connections    = backend.m_connections;

    gen_aliases();
}

void Settings::save() {
    SETTINGS_BACKEND backend(m_filename);
    backend.m_input_volumes  = m_input_volumes;
    backend.m_output_volumes = m_output_volumes;
    backend.m_connections    = m_connections;
    backend.save();
}

Settings::~Settings(){
    return;
}

template<typename TK, typename TV>
std::vector<TK> get_map_keys(std::map<TK, TV> const& input_map) {
    std::vector<TK> retval;
    for (const auto& p : input_map)
        retval.push_back(p.first);
    return retval;
}


///
/// Generate io name aliases
///
void Settings::gen_aliases() {
    m_input_aliases = {};
    m_output_aliases = {};
    int counter = 0;
    for (auto p : m_input_volumes) {
        m_input_aliases["I" + std::to_string(counter++)] = p.first;
    }
    counter = 0;
    for (auto p : m_output_volumes) {
        m_output_aliases["O" + std::to_string(counter++)] = p.first;
    }
}


///
/// Return true if input is alias
///
const bool Settings::is_input_alias(const std::string& input) {
    return (m_input_aliases.find(input) != m_input_aliases.end());
}

///
/// Return true if output is alias
///
const bool Settings::is_output_alias(const std::string& output) {
    bool out = (m_output_aliases.find(output) != m_output_aliases.end());
    /* std::cout << out << std::endl; */
    return out;
}


///
/// Get real name of input (return input if not found in aliases)
///
const std::string Settings::get_input_name(const std::string& input) {
    if (is_input_alias(input))
        return m_input_aliases[input];
    return input;
}

///
/// Get real name of output (return output if not found in aliases)
///
const std::string Settings::get_output_name(const std::string& output) {
    if (is_output_alias(output)) {
        return m_output_aliases[output];
    }
    return output;
}


///
/// Get vector containing all input aliases
///
const std::vector<std::string> Settings::get_input_aliases() {
    return get_map_keys(m_input_aliases);
}

///
/// Get vector containing all output aliases
///

const std::vector<std::string> Settings::get_output_aliases() {
    return get_map_keys(m_output_aliases);
}

///
/// Get vector containing the names of all inputs
///
const std::vector<std::string> Settings::get_inputs() {
    std::vector<std::string> inputs = get_map_keys(m_input_volumes);

    return inputs;
}

///
/// Get vector containing the names of all outputs
///
const std::vector<std::string> Settings::get_outputs() {
    std::vector<std::string> outputs = get_map_keys(m_output_volumes);

    return outputs;
}


///
/// Get vector containing the names of all inputs connected to output
///
const std::vector<std::string> Settings::get_connections(const std::string& output) {
    return m_connections[get_output_name(output)];
}


///
/// Check if input is in inputs
///
const bool Settings::is_input(const std::string& input, const bool check_alias) {
    std::string i;
    if (check_alias)
         i = get_input_name(input);
    else i = input;

    std::vector<std::string> ins = get_inputs();
    return (std::find(ins.begin(), ins.end(), i) != ins.end());
}

///
/// Check if output is in outputs
///
const bool Settings::is_output(const std::string& output, const bool check_alias) {
    std::string o;
    if (check_alias)
         o = get_output_name(output);
    else o = output;

    std::vector<std::string> outs = get_outputs();
    return (std::find(outs.begin(), outs.end(), o) != outs.end());
}


///
/// Add input to inputs with volume vol (default=1)
///
void Settings::add_input(const std::string& name, float vol) {
    m_input_volumes[name] = vol;

    gen_aliases();
}

///
/// Add output to outputs with volume vol (default=1)
///
void Settings::add_output(const std::string& name, float vol) {
    m_output_volumes[name] = vol;

    gen_aliases();
}


///
/// Remove input
///
void Settings::remove_input(const std::string& input) {
    m_input_volumes.erase(get_input_name(input));
}

///
/// Remove output
///
void Settings::remove_output(const std::string& output) {
    m_output_volumes.erase(get_output_name(output));
}


///
/// Get volume of input
///
const float Settings::get_input_volume(const std::string& input){
    const std::string i = get_input_name(input);
    if (!is_input(i))
        throw InputNotFound(i);

    return m_input_volumes[i];
}

///
/// Get volume of output
///
const float Settings::get_output_volume(const std::string& output){
    const std::string o = get_output_name(output);
    if (!is_output(o))
        throw OutputNotFound(o);

    return m_output_volumes[o];
}

///
/// Set volume of input
///
void Settings::set_input_volume(const std::string& input, float new_vol) {
    const std::string i = get_input_name(input);
    if (!is_input(i))
        throw InputNotFound(i);

    new_vol = new_vol > 1 ? 1 : new_vol;
    new_vol = new_vol < 0 ? 0 : new_vol;
    m_input_volumes[i] = new_vol;
}

///
/// Set volume of output
///
void Settings::set_output_volume(const std::string& output, float new_vol) {
    const std::string o = get_output_name(output);

    if (!is_output(o))
        throw OutputNotFound(o);

    new_vol = new_vol > 1 ? 1 : new_vol;
    new_vol = new_vol < 0 ? 0 : new_vol;
    m_output_volumes[o] = new_vol;
}

///
/// Connect input with output if not already connected
///
void Settings::connect(const std::string& input, const std::string& output) {
    const std::string i = get_input_name(input);
    const std::string o = get_output_name(output);

    if (!is_input(i))
        throw InputNotFound(i);
    if (!is_output(o))
        throw OutputNotFound(o);

    std::vector<std::string> output_connections = m_connections[o];
    if (find(begin(output_connections), end(output_connections), i) == end(output_connections))
        m_connections[o].push_back(i);
}

///
/// Disconnect input and output if not already disconnected
///
void Settings::disconnect(const std::string& input, const std::string& output) {
    const std::string i = get_input_name(input);
    const std::string o = get_output_name(output);

    if (!is_input(i))
        throw InputNotFound(i);
    if (!is_output(o))
        throw OutputNotFound(o);

    m_connections[o].erase(std::find(m_connections[o].begin(), m_connections[o].end(), i));
}


///
/// Return true if input and output are connected
///
const bool Settings::is_connected(const std::string& input, const std::string& output) {
    const std::string i = get_input_name(input);
    const std::string o = get_output_name(output);

    if (!is_input(i))
        throw InputNotFound(i);
    if (!is_output(o))
        throw OutputNotFound(o);

    return (std::find(m_connections[o].begin(), m_connections[o].end(), i) != m_connections[o].end());
}
