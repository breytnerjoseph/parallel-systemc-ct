#ifndef PARALLELIZATION_ALGORITHM_H
#define PARALLELIZATION_ALGORITHM_H

// #include <iostream>
// #include <iomanip>
#include <vector>
#include <thread>
#include <algorithm>
#include "ode_system.h"
#include "thread_pool.h"
#include <future>
#include "parallel_cluster.h"
#include "synchronization_layer.h"

//  Intel TBB 
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"


#define USE_TBB 1

// #define USE_REGISTERING_PROCESS


// Singleton 
namespace sct_kernel{ 

typedef std::vector<sct_kernel::ode_system *> ode_system_container;


////////////////////////////////////////////////////////////////////////////////
// TBB 
////////////////////////////////////////////////////////////////////////////////

class ProcessExecutor {
    ode_system_container *processes;

    public:

        void operator() ( const tbb::blocked_range<size_t>& r ) const { 
 
            for (auto i = r.begin(); i != r.end(); ++i) {
                processes->operator[](i)->execute();
            }

        }

    ProcessExecutor(ode_system_container *processes) : processes(processes) { }
};


class OutputGenerator {
    ode_system_container *processes;

    public:

        void operator() ( const tbb::blocked_range<size_t>& r ) const { 
 
            for (auto i = r.begin(); i != r.end(); ++i) {
                processes->operator[](i)->generate_outputs();
            }

        }

    OutputGenerator(ode_system_container *processes) : processes(processes) { }
};



class parallelization_algorithm {

    public:
        // Method to get the singleton instance
        static parallelization_algorithm &get() noexcept;
        

        // Singleton methods
        void store_module(sct_kernel::ode_system *);
        void add_to_parallel_runnable_processes(sct_kernel::ode_system *);
        void start_of_simulation();
        void end_of_simulation();
        void print_exec_time();
        

        // Avoid copies
        parallelization_algorithm(const parallelization_algorithm&) = delete;
        parallelization_algorithm& operator=(const parallelization_algorithm&) = delete;

        

    private: 
        // Singleton manager to assure 
        // automatic destruction
        class manager {
            public:
                manager();

                ~manager() noexcept;

                sct_kernel::parallelization_algorithm *get();
                
            private: 
                sct_kernel::parallelization_algorithm *singleton;
        };

        static manager my_manager;
        // Private constructor
        parallelization_algorithm();

        // Singleton attributes
        // Set of ode systems that is ordered by the next time of execution
        ode_system_container ode_systems; 
        ode_system_container processes_to_run;

        // Signals whether the systemc process has been launched
        // so that it is not launched twice
        bool launched;

        // Signals execution during SystemC's initialization phase
        // Used to execute all ct processes
        bool initialization_phase;

        // List of events to keep sensitivity on inputs
        sc_core::sc_event_or_list input_sensitivity_list;

        // // SINGLETON METHODS
        void parallelization_process();
        void create_systemc_process();
        void create_sensitivity_list();
        void execute_processes(ode_system_container); 
        void schedule_reactivation();
        sc_core::sc_time next_reactivation_time();
        sc_core::sc_event added_new_process_ev;
        void get_processes_to_run();
        void set_static_sensitivity(sc_core::sc_spawn_options &);

        double exec_time;
        double exec_time2;
        double avg_proc;


        // std::vector< std::function<void()> is_event_func;


        // // THREAD POOL 
        thread_pool *pool;


        // PARALLELIZATION WITH CLUSTERING STRATEGY
        parallel_cluster cluster;
        void initialize_parallel_clusters();

        
};

} //  namespace sct_kernel

#endif