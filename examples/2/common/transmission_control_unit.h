#ifndef TRANSMISSION_CONTROL_UNIT_H
#define TRANSMISSION_CONTROL_UNIT_H

#include <systemc>
#include "shifting_thresholds.h"
#include "state_selector.h"
#include "gear_state.h"

class TransmissionControlUnit : public sc_core::sc_module {
    SC_HAS_PROCESS(TransmissionControlUnit);
    
    public: 
        // Ports
        sc_core::sc_port<sc_core::sc_event_queue_if> update_in;
        sc_core::sc_in<ShiftingThresholds> threshold_in;
        sc_core::sc_out<double> gear_out;
        sc_core::sc_out<double> gear_ratio_out;

        TransmissionControlUnit(sc_core::sc_module_name name);
    
    private:
        StateSelector state_selector;
        GearState gear_state;

        void check_thresholds();

        sc_core::sc_signal<bool> direction_sig;
        sc_core::sc_event_queue update_gear_evq;
};

#endif