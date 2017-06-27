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
    return m_connections[output];
}


///
/// Check if input is in inputs
///
const bool Settings::is_input(const std::string& input) {
    std::vector<std::string> ins = get_inputs();
    return (std::find(ins.begin(), ins.end(), input) != ins.end());
}

///
/// Check if output is in outputs
///
const bool Settings::is_output(const std::string& output) {
    std::vector<std::string> outs = get_outputs();
    return (std::find(outs.begin(), outs.end(), output) != outs.end());
}


///
/// Add input to inputs with volume vol (default=1)
///
void Settings::add_input(const std::string& name, float vol) {
    m_input_volumes[name] = vol;
}

///
/// Add output to outputs with volume vol (default=1)
///
void Settings::add_output(const std::string& name, float vol) {
    m_output_volumes[name] = vol;
}


///
/// Remove input
///
void Settings::remove_input(const std::string& input) {
    m_input_volumes.erase(input);
}

///
/// Remove output
///
void Settings::remove_output(const std::string& output) {
    m_output_volumes.erase(output);
}


///
/// Get volume of input
///
const float Settings::get_input_volume(const std::string& input){
    if (is_input(input))
        return m_input_volumes[input];
    else
        throw InputNotFound(input);
}

///
/// Get volume of output
///
const float Settings::get_output_volume(const std::string& output){
    if (is_output(output))
        return m_output_volumes[output];
    else
        throw OutputNotFound(output);
}

///
/// Set volume of input
///
void Settings::set_input_volume(const std::string& input, float new_vol) {
    m_input_volumes[input] = new_vol;
}

///
/// Set volume of output
///
void Settings::set_output_volume(const std::string& output, float new_vol) {
    m_output_volumes[output] = new_vol;
}

///
/// Connect input with output if not already connected
///
void Settings::connect(const std::string& input, const std::string& output) {
    std::vector<std::string> output_connections = m_connections[output];
    if (find(begin(output_connections), end(output_connections), input) == end(output_connections))
        m_connections[output].push_back(input);
}

///
/// Disconnect input and output if not already disconnected
///
void Settings::disconnect(const std::string& input, const std::string& output) {
    m_connections[output].erase(std::find(m_connections[output].begin(), m_connections[output].end(), input));
}


///
/// Return true if input and output are connected
///
const bool Settings::is_connected(const std::string& input, const std::string& output) {
    return (std::find(m_connections[output].begin(), m_connections[output].end(), input) != m_connections[output].end());
}
