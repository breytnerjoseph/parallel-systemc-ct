#include<systemc>
#include<iostream> 
#include <string>
#include <chrono>

#include "sync_conf_utility.h"

#include "switch_driver_de.h"
#include "circuit_sc_module.h"




// Pass as first parameter y to use the adaptive sync. strategy,
// y as second parameter to avoid rollbacks (use time of the next
// event), and a real number as the third parameter to set the timestep
int sc_main(int argc, char* argv[]){

    auto t1 = std::chrono::high_resolution_clock::now();

    sc_core::sc_time simulated_time(100, sc_core::SC_SEC);
    bool use_adaptive = true;
    bool avoid_rollback = false;
    double timestep = 0;
    bool parallel_execution = false;

    get_sync_conf(argc, argv, use_adaptive, avoid_rollback, timestep, parallel_execution);


    sc_core::sc_signal<double> v_sg_1, v_sg_2;
    sc_core::sc_signal<bool> switch_sg_1, switch_sg_2, up_sg_1, down_sg_1, up_sg_2, down_sg_2;
    sc_core::sc_event_queue ev_q;

    SwitchDriverDE switch_driver1("driver1");


    int nb_circuits = 32;

    for (int i = 0; i < nb_circuits; i++) {
        
        CircuitScModule *circuit_ptr = new CircuitScModule( (std::string("circuit") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);

        sc_core::sc_signal<double> *v_sg_ptr = new sc_core::sc_signal<double>;
        sc_core::sc_signal<bool> *up_sg_ptr = new sc_core::sc_signal<bool>;
        sc_core::sc_signal<bool> *down_sg_ptr = new sc_core::sc_signal<bool>;
        
        circuit_ptr->v_out.bind(*v_sg_ptr);
        circuit_ptr->switch_in.bind(switch_sg_1);
        circuit_ptr->up_out.bind(*up_sg_ptr);
        circuit_ptr->down_out.bind(*down_sg_ptr);

        // Connect driver circuit
        if (i == 0) {
            switch_driver1.up_in.bind(*up_sg_ptr);
            switch_driver1.down_in.bind(*down_sg_ptr);
        }

    }

    switch_driver1.switch_out.bind(switch_sg_1);


    // Start simulation for a given time
    sc_core::sc_start(simulated_time);

    // Finish simulation
    sc_core::sc_stop();
 
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();



    print_sync_stats(use_adaptive, avoid_rollback, timestep, simulated_time.to_seconds(), duration * 1e-06);

    return 0;
}