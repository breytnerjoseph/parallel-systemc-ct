#ifndef CT_MODULE_H
#define CT_MODULE_H

#include <systemc>

#include "ode_system.h"
#include "synchronization_layer.h"
#include "parallelization_algorithm.h"


// THESE ARE NOT SOLVER STEPS, BUT INTEGRATION INTERVAL LENGTHS
// Minimum allowed integration interval length
#define MIN_DELTA_T 0.00000001 // seconds.  
// Default synchronization step.
#define DEFAULT_DELTA_T 0.1 // seconds.

// This class abstracts out common operations 
// in all CT Modules : 
// 1. Creation of a synchronization layer object
// 2. Setting up of the initial conditions of the system

namespace sct_core{  
    class ct_module : public sc_core::sc_module, public sct_kernel::ode_system {
        public:
            ct_module(
                sc_core::sc_module_name name = sc_core::sc_gen_unique_name("ct_module"), 
                bool use_adaptive_sync_step = true, bool use_time_to_next_event = false, bool parallel_execution = false );

            // TO BE CALLED AUTOMATICALLY
            void end_of_elaboration();
            void end_of_simulation();

            ///////////////////////////////////////////
            // USED IN THE PARALLELIZATION ALGORITHM
            ///////////////////////////////////////////
            const sc_core::sc_time &get_local_time() const; 
            const char *get_proc_name() const;
            void start_of_simulation();

            // To be implemented by the user, 
            // as in TDF modules of SystemC AMS
            // e.g. call set_max_timestep
            // The default behavior is to set 
            // a default maximum timestep
            virtual void set_sync_parameters();

            // Set the maximum allowed synchronization timestep
            void set_max_timestep(double max);

            void set_initial_conditions();


            void use_adaptive_sync(bool);
            void avoid_rollbacks(bool);

            bool should_generate_outputs();
            void generate_outputs();

            //////////////////////////////////////////////////////////
            // USED IN THE PARALLELIZATION WITH CLUSTERING STRATEGY
            //////////////////////////////////////////////////////////
            virtual sct_kernel::synchronization_layer *get_sync_layer();

        private:
            double max_step;
            bool use_adaptive_sync_step;
            bool use_time_to_next_event;
            sct_kernel::synchronization_layer *sync_layer;
            bool parallel_execution;

            // Execute the module in a interval
            void execute();
            // Adds the process to the set of parallel runnable processes in the current delta cycle
            void add_to_parallel_runnable_processes();
            void notify_finished_exec();

    };
}

#endif