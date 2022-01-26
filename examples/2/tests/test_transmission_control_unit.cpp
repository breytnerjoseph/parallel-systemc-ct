#include<systemc>
#include<iostream>

#include "transmission_control_unit.h"
#include "transmission_control_unit_tb.h"
#include "shifting_thresholds.h"

int sc_main(int argc, char *argv[]){

    TransmissionControlUnit tcu("tcu");
    TcuTb tb("tb");
    

    sc_core::sc_signal<int> gear_sig("gear_sig");
    sc_core::sc_signal<double> gear_ratio_sig("gear_ratio_sig");
    sc_core::sc_signal<ShiftingThresholds> threshold_sig("threshold_sig");
    sc_core::sc_event_queue up_evq("up_evq");

    tb.update_out(up_evq);
    tb.threshold_out(threshold_sig);
    tb.gear_in(gear_sig);
    
    tcu.update_in(up_evq);
    tcu.threshold_in(threshold_sig);
    tcu.gear_out(gear_sig);
    tcu.gear_ratio_out(gear_ratio_sig);

    sc_core::sc_start();

    return 0;
}