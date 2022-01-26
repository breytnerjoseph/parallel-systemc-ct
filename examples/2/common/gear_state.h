#ifndef GEAR_STATE_H
#define GEAR_STATE_H

#include <systemc>


class GearState : public sc_core::sc_module {
    SC_HAS_PROCESS(GearState);
    
    public: 
        // Ports
        sc_core::sc_port<sc_core::sc_event_queue_if> update_in;
        sc_core::sc_in<bool> direction_in;
        sc_core::sc_out<double> gear_out;
        sc_core::sc_out<double> gear_ratio_out;

        GearState(sc_core::sc_module_name name);
    
    private:
        enum StateEnum {first = 1, second, third, fourth};
        StateEnum state;
        static const double gear_ratios[4];
        int activations_count;

        void update_gear();
        void increase_gear();
        void decrease_gear();
        void generate_output();

        void end_of_simulation();
};

#endif