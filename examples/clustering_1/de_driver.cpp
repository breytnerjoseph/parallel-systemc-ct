#include "de_driver.h"

DeDriver::DeDriver(
    sc_core::sc_module_name name,
    double mean,
    double standard_deviation) 
    : distribution(mean, standard_deviation) 
{
    auto name_str = std::string(name);
    std::seed_seq seed (name_str.begin(), name_str.end());
    generator.seed(seed);

    SC_THREAD(control_loop);
    event_out.initialize(true);
}

void DeDriver::control_loop() {

    while (true) {
        sc_core::wait(get_time_to_next_event());
        event_out.write(!event_out.read());
    }
}

sc_core::sc_time DeDriver::get_time_to_next_event() {
    double time_to_next_event = distribution(generator);
    return sc_core::sc_time(time_to_next_event, sc_core::SC_SEC);
}
