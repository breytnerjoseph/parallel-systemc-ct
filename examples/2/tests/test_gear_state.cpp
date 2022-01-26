#include<systemc>
#include<iostream>

#include "gear_state.h"
#include "gear_state_tb.h"



int sc_main(int argc, char *argv[]){

    GearState gearState("gearState");
    GearStateTb tb("tb");

    sc_core::sc_signal<int> gear_sig("gear_sig");
    sc_core::sc_signal<double> gear_ratio_sig("gear_ratio_sig");
    sc_core::sc_signal<bool> direction_sig("direction_sig");
    sc_core::sc_event_queue up_evq("up_evq");

    tb.update_out(up_evq);
    tb.direction_out(direction_sig);
    tb.gear_in(gear_sig);
    tb.gear_ratio_in(gear_ratio_sig);

    gearState.update_in(up_evq);
    gearState.direction_in(direction_sig);
    gearState.gear_out(gear_sig);
    gearState.gear_ratio_out(gear_ratio_sig);

    sc_core::sc_start();

    return 0;
}