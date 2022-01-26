#ifndef STATE_SELECTOR_H
#define STATE_SELECTOR_H

#include <systemc>
#include "shifting_thresholds.h"

class StateSelector : public sc_core::sc_module {
    SC_HAS_PROCESS(StateSelector);
    
    public: 
        // Ports
        sc_core::sc_port<sc_core::sc_event_queue_if> update_in;
        sc_core::sc_in<ShiftingThresholds> threshold_in;
        // Note that we can define a primitive channel to transmit 
        // valued events, like the signal channel but that 
        // reacts to the write event rather than to the value_changed event
        sc_core::sc_port<sc_core::sc_event_queue_if> update_out; 
        sc_core::sc_out<bool> direction_out;

        StateSelector(sc_core::sc_module_name name);
    
    private:
        enum StateEnum {steady, downshifting, upshifting};
        StateEnum state; 
        sc_core::sc_event output_timer;

        int activations_count;

        void update_state();
        void generate_output();

        void end_of_simulation();
};

#endif