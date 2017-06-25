#include <iostream>

#include <chrono>

#include "backend.h"

///
/// Constructor:
///     @param client_name the display name of the jack client
///
Backend::Backend(const std::string client_name) : m_client_name(client_name),
                                                  settings(Settings(CONFIG_PATH)) {
    settings.load();
}

///
/// setup jack client, set callbacks, create ports,
///
void Backend::setup() {

    // Open client
    m_client = jack_client_open(m_client_name.c_str(), JackNoStartServer, NULL);
    /* m_client = jack_client_new(m_client_name.c_str()); */
    /* ASSERT(m_client == 0, "Jack server not running?"); */
    if (m_client == 0)
        throw JackServerIsDown();

    // === Set callbacks ===
    //
    // Since the callbacks are member functions, we
    // need to create intermediate anonymous functions
    //
    // = Set process callback =
    int (*process_callback)(jack_nframes_t, void*) = [](jack_nframes_t n, void* b){
            return ((Backend *)b)->callback(n);
        };
    jack_set_process_callback(m_client, process_callback, this);

    // = Set shutdown callback =
    void (*shutdown_callback)(void*) = [](void* b){
            std::cout << "Server is shutting down..." << std::endl;
            ((Backend *)b)->m_client = 0;
    };
    jack_on_shutdown(m_client, shutdown_callback, this);

    // === Register ports ===
    // Inputs
    for (std::string i : settings.get_inputs()) {
        register_port(i, true);
    }
    // Outputs
    for (std::string o : settings.get_outputs()) {
        register_port(o, false);
    }

    // === Activate client ===
    int active = jack_activate(m_client);
    ASSERT(active, "Could not activate client")
}

///
/// Reconnection to jack in case the server stops
///
void Backend::start_recon_loop() {
    auto recon = [this](){
        while (m_try_recon) {
            if (!m_client) {
                try {
                    std::cout << "Attempting reconnection..." << std::endl;
                    setup();
                } catch (JackServerIsDown& e) {
                    std::cout << "Jack server is down" << std::endl;
                }
            }
            /* std::cout << m_client << std::endl; */
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    };

    m_try_recon = true;
    m_recon_loop = std::thread(recon);
}

///
/// Stop reconnection loop
///
void Backend::stop_recon_loop() {
    m_try_recon = false;
    m_recon_loop.join();
}

///
/// Register a new jack port. (Also adds the input/ouput to settings in needed)
///
void Backend::register_port(const std::string name, const bool input) {
    if (input) {
        // explicit input:
        m_input_ports[name] = {
            jack_port_register(m_client, (name+LEFT_SUFFIX ).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0),
            jack_port_register(m_client, (name+RIGHT_SUFFIX).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0),
        };
        // implicit output:
        m_implicit_output_ports[name] = {
            jack_port_register(m_client, (name+OUT_SUFFIX+LEFT_SUFFIX ).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
            jack_port_register(m_client, (name+OUT_SUFFIX+RIGHT_SUFFIX).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
        };
        if (!settings.is_input(name)) {
            settings.add_input(name, 1);
        }
    } else {
        // explicit output:
        m_explicit_output_ports[name] = {
            jack_port_register(m_client, (name+LEFT_SUFFIX ).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
            jack_port_register(m_client, (name+RIGHT_SUFFIX).c_str(), JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0),
        };
        if (!settings.is_output(name)) {
            settings.add_output(name, 1);
        }
    }
}

///
/// Unregister port and remove input/output from settings
///
void Backend::unregister_port(const std::string name, const bool input) {
    if (input) {
        jack_port_unregister(m_client, m_input_ports[name][0]);
        jack_port_unregister(m_client, m_input_ports[name][1]);
        jack_port_unregister(m_client, m_implicit_output_ports[name][0]);
        jack_port_unregister(m_client, m_implicit_output_ports[name][1]);

        m_input_ports.erase(name);
        m_implicit_output_ports.erase(name);
        settings.remove_input(name);
    } else {
        jack_port_unregister(m_client, m_explicit_output_ports[name][0]);
        jack_port_unregister(m_client, m_explicit_output_ports[name][1]);

        m_explicit_output_ports.erase(name);
        settings.remove_output(name);
    }
}

///
/// Rename port in jack backend and in settings
///
void Backend::rename_port(const std::string old_name, const std::string new_name, const bool input) {
    if (input) {
        float old_vol = settings.get_input_volume(old_name);
        settings.remove_input(old_name);

        m_input_ports[new_name] = m_input_ports[old_name];
        m_input_ports.erase(old_name);

        m_implicit_output_ports[new_name] = m_implicit_output_ports[old_name];
        m_implicit_output_ports.erase(old_name);

        jack_port_set_name(m_input_ports[new_name][0], (new_name+LEFT_SUFFIX ).c_str());
        jack_port_set_name(m_input_ports[new_name][1], (new_name+RIGHT_SUFFIX).c_str());

        jack_port_set_name(m_implicit_output_ports[new_name][0], (new_name+OUT_SUFFIX+LEFT_SUFFIX ).c_str());
        jack_port_set_name(m_implicit_output_ports[new_name][1], (new_name+OUT_SUFFIX+RIGHT_SUFFIX).c_str());

        settings.add_input(new_name, old_vol);
    } else {
        float old_vol = settings.get_output_volume(old_name);
        settings.remove_output(old_name);

        m_explicit_output_ports[new_name] = m_explicit_output_ports[old_name];
        m_explicit_output_ports.erase(old_name);

        jack_port_set_name(m_explicit_output_ports[new_name][0], (new_name+LEFT_SUFFIX ).c_str());
        jack_port_set_name(m_explicit_output_ports[new_name][1], (new_name+RIGHT_SUFFIX).c_str());

        settings.add_output(new_name, old_vol);
    }
}

///
/// Callback function. Does the actual backend audio connection handling.
///
int Backend::callback(jack_nframes_t nframes) {
# define SAMPLE_TYPE jack_default_audio_sample_t

    // Mirror inputs to output versions (with volume controls)
    for (const auto& in : m_input_ports) {
        std::vector<jack_port_t*> out = m_implicit_output_ports[in.first];

        SAMPLE_TYPE* oleft  = (SAMPLE_TYPE*) jack_port_get_buffer(out[0], nframes);
        SAMPLE_TYPE* oright = (SAMPLE_TYPE*) jack_port_get_buffer(out[1], nframes);

        SAMPLE_TYPE* ileft  = (SAMPLE_TYPE*) jack_port_get_buffer(in.second[0], nframes);
        SAMPLE_TYPE* iright = (SAMPLE_TYPE*) jack_port_get_buffer(in.second[1], nframes);

        float volume_mod = settings.get_input_volume(in.first);
        for (jack_nframes_t i=0; i<nframes; i++) {
            oleft[i]  = ileft[i]  * volume_mod;
            oright[i] = iright[i] * volume_mod;
        }
    }

    // Add inputs to the outputs theyre connected to with volume mod for each
    for (const auto& out : m_explicit_output_ports) {
        SAMPLE_TYPE* oleft  = (SAMPLE_TYPE*) jack_port_get_buffer(out.second[0], nframes);
        SAMPLE_TYPE* oright = (SAMPLE_TYPE*) jack_port_get_buffer(out.second[1], nframes);

        // Set output to 0
        for (jack_nframes_t i=0; i<nframes; i++) {
            oleft[i]  = 0;
            oright[i] = 0;
        }

        // Add connected inpus with their volume mods
        for ( std::string iname : settings.get_connections(out.first)) {
            /* cout << o.first << iname << endl; */
            std::vector<jack_port_t*> in = m_input_ports[iname];

            SAMPLE_TYPE* ileft  = (SAMPLE_TYPE*) jack_port_get_buffer(in[0], nframes);
            SAMPLE_TYPE* iright = (SAMPLE_TYPE*) jack_port_get_buffer(in[1], nframes);

            float ivolume_mod = settings.get_input_volume(iname);
            for (jack_nframes_t i=0; i<nframes; i++) {
                oleft[i]  += ileft[i]  * ivolume_mod;
                oright[i] += iright[i] * ivolume_mod;
            }
        }

        // Apply output volume_mod
        float ovolume_mod = settings.get_output_volume(out.first);
        for (jack_nframes_t i=0; i<nframes; i++) {
            oleft[i]  *= ovolume_mod;
            oright[i] *= ovolume_mod;
        }
    }

    return 0;
#undef SAMPLE_TYPE
}

///
/// Close jack client
///
void Backend::shutdown() {
    if (m_client) {
        std::cout << "Shutting down client..." << std::endl;
        jack_deactivate(m_client);
        jack_client_close(m_client);
        m_client = 0;
    }
}

///
/// Destructor
///
Backend::~Backend() {
    // its important to properly close the client
    // but only if it hasn't been closed already!
    if (m_client)
        shutdown();
}
