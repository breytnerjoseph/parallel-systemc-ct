#ifndef DE_DRIVER_H
#define DE_DRIVER_H

#include <systemc>
#include <chrono>
#include <random>

#define DEFAULT_MEAN 0.5
#define DEFAULT_STANDARD_DEVIATION 0.05

// Module that generates
// the switch control signal
class DeDriver : sc_core::sc_module {

    public:
        SC_HAS_PROCESS(DeDriver);

        DeDriver(
            sc_core::sc_module_name = sc_core::sc_gen_unique_name("de_driver"), double = DEFAULT_MEAN, 
            double = DEFAULT_STANDARD_DEVIATION);

        sc_core::sc_out<bool> event_out;
        
        void control_loop();
        sc_core::sc_time get_time_to_next_event();

    private:
        std::default_random_engine generator;
        std::normal_distribution<double> distribution;

};

#endif