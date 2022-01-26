#ifndef PARALLLEL_CLUSTER_H
#define PARALLLEL_CLUSTER_H

#include <set>
#include <algorithm>
#include <systemc>
#include "ode_system.h"
#include "synchronization_layer.h"

#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"

#define STEP_COEFF 0.3

class parallel_cluster {
    
    typedef std::vector<sct_kernel::ode_system *> module_container;

    public: 
        parallel_cluster();
        bool add_module(sct_kernel::ode_system *);
        bool remove_module(sct_kernel::ode_system *);
        void execute();
        sc_core::sc_time get_time();
        void generate_outputs();

        // TO COMPLETE
        void initialize();

        // Utility
        sc_core::sc_time get_next_cluster_time();
        void synchronize_modules(sc_core::sc_time);
        sc_core::sc_time get_current_cluster_time();
        void execute_parallel(const module_container &, sc_core::sc_time);
        bool has_been_activated_by_input();
        void get_global_time();
        bool is_delta_cycle() const;
        bool cluster_t_greater_than_global_t() const;
        module_container get_modules_activated_by_input() const;
        void execute_and_synchronize(const module_container &, sc_core::sc_time);
        void estimate_time_between_inputs();
        double get_avg_processes() const;


    private: 
       
        module_container modules;

        sc_core::sc_time t_cluster, estimated_time_between_inputs, last_input_event_time, global_time, last_activation_time; 
        
        double avg_processes;

};


#endif