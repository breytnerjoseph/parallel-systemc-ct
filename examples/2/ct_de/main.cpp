#include <iostream>
#include <fstream>
#include <string>
#include <systemc>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

#include "sync_conf_utility.h"
#include "vehicle_ct.h"
#include "transmission_control_unit.h"




// Pass as first parameter y to use the adaptive sync. strategy,
// y as second parameter to avoid rollbacks (use time of the next
// event), and a real number as the third parameter to set the timestep
int sc_main(int argc, char *argv[]){
    // auto t1 = std::chrono::high_resolution_clock::now();
    auto t1 = std::chrono::steady_clock::now();

    sc_core::sc_time simulated_time(30, sc_core::SC_SEC);
    bool use_adaptive = false;
    bool avoid_rollback = false;
    double timestep = 0.01;
    bool parallel_execution = false;

    get_sync_conf(argc, argv, use_adaptive, avoid_rollback, timestep, parallel_execution);

    sc_core::sc_signal<double> gear_sig("gear_sig");
    sc_core::sc_signal<double> gear_ratio_sig("gear_ratio_sig");
   
    TransmissionControlUnit tcu("tcu");
    tcu.gear_out(gear_sig);
    tcu.gear_ratio_out(gear_ratio_sig);


    int nb_ct_modules = 64;

    for (int i = 0; i < nb_ct_modules; i++) {
        VehicleCt *vehicle_ptr = new VehicleCt((std::string("vehicle") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);
        
        sc_core::sc_signal<bool> *crossing_sg_ptr = new sc_core::sc_signal<bool>;
        
        sc_core::sc_event_queue *up_evq_ptr = new sc_core::sc_event_queue();    
        sc_core::sc_signal<ShiftingThresholds> *threshold_sig_ptr = new sc_core::sc_signal<ShiftingThresholds>();
        sc_core::sc_signal<double> *speed_sig_ptr = new sc_core::sc_signal<double>();

        vehicle_ptr->update_out(*up_evq_ptr);
        vehicle_ptr->threshold_out(*threshold_sig_ptr);
        vehicle_ptr->speed_out(*speed_sig_ptr);

        vehicle_ptr->gear_in(gear_sig);
        vehicle_ptr->gear_ratio_in(gear_ratio_sig);

        // Connect driver circuit
        if (i == 0) {
            tcu.update_in(*up_evq_ptr);
            tcu.threshold_in(*threshold_sig_ptr);
        }

    }


    sc_start(simulated_time);
    sc_core::sc_stop();
 
    auto t2 = std::chrono::steady_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();



    print_sync_stats(use_adaptive, avoid_rollback, timestep, simulated_time.to_seconds(), duration * 1e-06);



    return 0;
}