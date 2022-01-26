#include<systemc>
#include<iostream> 
#include <string>
#include <chrono>
#include <stdlib.h>
#include "tbb/task_scheduler_init.h"

#include "de_driver.h"
#include "ct_test_module.h"

#include "sync_conf_utility.h"


void parallel_args(int argc, char *argv[], bool &is_parallel, int &nb_modules, int &nb_threads) {

    if (argc == 4) {
        is_parallel = std::string(argv[1]) == std::string("y");
        nb_modules = atoi(argv[2]);
        nb_threads = atoi(argv[3]);
    }
}


// Pass as first parameter y to use the adaptive sync. strategy,
// y as second parameter to avoid rollbacks (use time of the next
// event), and a real number as the third parameter to set the timestep
int sc_main(int argc, char* argv[]){

    auto t1 = std::chrono::high_resolution_clock::now();

    sc_core::sc_time simulated_time(300, sc_core::SC_SEC);
    bool use_adaptive = true;
    bool avoid_rollback = false;
    double timestep = 0;
    bool parallel_execution = false;
    int nb_modules = 2;
    int nb_threads = 4;

    parallel_args(argc, argv, parallel_execution, nb_modules, nb_threads);

    // Set TBB number of threads 
    tbb::task_scheduler_init init(nb_threads);

    double time_between_events_mean = 100;
    double time_between_events_standard_deviation = 10; 

    CtTestModule modules[nb_modules];

    for (int i = 0; i < nb_modules; i++) {
        
        DeDriver *driver_ptr = new DeDriver( 
            (std::string("de_driver") +  std::to_string(i)).c_str(),
            time_between_events_mean,
            time_between_events_standard_deviation );

        // CtTestModule *circuit_ptr = new CtTestModule( (std::string("ct_test_module") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);

        sc_core::sc_signal<bool> *ev_sg_ptr = new sc_core::sc_signal<bool>;

        driver_ptr->event_out.bind(*ev_sg_ptr);       
        modules[i].switch_in.bind(*ev_sg_ptr);
    }

    // Start simulation for a given time
    sc_core::sc_start(simulated_time);

    // Finish simulation
    sc_core::sc_stop();
 
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();


    print_sync_stats(use_adaptive, avoid_rollback, timestep, simulated_time.to_seconds(), duration * 1e-06);

    return 0;
}