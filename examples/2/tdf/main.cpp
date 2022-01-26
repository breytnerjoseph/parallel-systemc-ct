#include <iostream>
#include <fstream>
#include <string>
#include <systemc>
#include <systemc-ams> /* Trace utilities */ 
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

#include "vehicle_tdf.h"
#include "transmission_control_unit.h"
#include "bool_to_event_converter.h"

int sc_main(int argc, char *argv[]){
    clock_t t = clock();

    VehicleTdf vehicle_tdf("vehicle_tdf");
    TransmissionControlUnit tcu("tcu");
    BoolToEventConverter bool_conv("bool_conv");

    sc_core::sc_signal<double> gear_sig("gear_sig"), speed_sig("speed");
    sc_core::sc_signal<double> gear_ratio_sig("gear_ratio_sig");
    sc_core::sc_signal<bool> up_bool_sig("up_bool_sig");
    sc_core::sc_signal<ShiftingThresholds> threshold_sig("threshold_sig");
    sc_core::sc_event_queue up_evq("up_evq");

    vehicle_tdf.update_out(up_bool_sig);
    vehicle_tdf.threshold_out(threshold_sig);
    vehicle_tdf.gear_in(gear_sig);
    vehicle_tdf.gear_ratio_in(gear_ratio_sig);
    vehicle_tdf.speed_out(speed_sig);

    bool_conv.bool_in(up_bool_sig);
    bool_conv.event_out(up_evq);

    tcu.update_in(up_evq);
    tcu.threshold_in(threshold_sig);
    tcu.gear_out(gear_sig);
    tcu.gear_ratio_out(gear_ratio_sig);


    sca_util::sca_trace_file *atf;
    atf = sca_util::sca_create_tabular_trace_file("speed.dat");
    sca_trace(atf, speed_sig, "v");
    sca_trace(atf, gear_sig, "gear");


    sc_start(30, sc_core::SC_SEC);
    sc_core::sc_stop();

    sca_util::sca_close_tabular_trace_file(atf);

    t = clock() - t;
    printf ("It took me %d clicks (%f seconds).\n", (int) t,((float)t)/CLOCKS_PER_SEC);

    return 0;
}