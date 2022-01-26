#ifndef SYNCHRONIZATION_LAYER_H
#define SYNCHRONIZATION_LAYER_H

#include <utility>
#include <systemc>
#include <time.h>
#include <fstream>


#define MAX_TIME_STEP 0.01 // seconds

// Coefficient for the average time between inputs 
// used in the calculation of the adaptive sync. step
// 1.30 has been found experimentally with various 
// use cases to be near the optimum, i.e., it 
// produces something near the speediest simulation
#define ADAPTIVE_STEP_AVG_COEFF 1.0



// Turn on to collect some internal statistics
// for development purposes, such as the synchronization step
// #define WRITE_STATISTICS_TO_FILE 0
#define STATISTICS_FILENAME "sync_statistics_data"

// #define COLLECT_SIM_STATISTICS 

// #define DEBUG

#include "numerical_solver.h"
#include "ode_system.h"
#include "ct_state.h"


// #define USE_REGISTERING_PROCESS

namespace sct_kernel {
    class synchronization_layer {
        public:
            synchronization_layer(ode_system *model_ptr, double max_timestep = MAX_TIME_STEP, bool use_adaptive_sync_step = true, bool use_time_to_next_event = false, bool parallel_execution = true);
            void store_input_event_sensitivity_list(const sc_core::sc_event_or_list &list);

            int get_backtrack_count();

            double get_handle_reactivation_sim_time() const;
            double get_calculate_solutions_sim_time() const;
            double get_t_aux() const;

            //////////////////////////////////////////////////
            // USED IN PARALLELIZATION
            //////////////////////////////////////////////////
            const sc_core::sc_time &get_local_time() const;
            void synchronization_process();
            void execute();

            bool should_generate_outputs();
            void generate_outputs();
            void notify_finished_exec() {
                finished_execution_ev.notify();
            }

            //////////////////////////////////////////////////////////
            // USED IN THE PARALLELIZATION WITH CLUSTERING STRATEGY
            //////////////////////////////////////////////////////////
            void set_sync_step(double);
            void set_adaptive_sync(bool);
            void set_sync_on_next_event(bool);
            double get_sync_step() const;
            bool has_been_activated_by_input();
            void synchronize(sc_core::sc_time t);
            double get_max_timestep() const;


        private:
            ////////////////////////////////////////////////////////
            // SIM STATISTICS
            /////////////////////////////////////////////////////////
            double t_handle_reactivation;
            double t_calculate_solutions;
            double t_schedule_reactivation;
            double t_aux;

            /////////////////////////////////////////////////////////
            // PRIVATE ATTRIBUTES
            /////////////////////////////////////////////////////////
            // Sync. step options
            bool use_adaptive_sync_step;
            bool use_time_to_next_event;

            // 
            bool parallel_execution;
            
            // Pointer to system model
            ode_system *system_ptr;

            // Indicate threshold crossing detection
            bool state_event_detected, state_event_located;
            bool catching_up;
            
            // Time variables
            sc_core::sc_time local_time, local_time_backup, global_time, last_activation_time;
            double sync_step, max_timestep; // No need to store it as a sc time
            double last_step;
            // For use in has_been_activated_by_input
            sc_dt::uint64 last_call_time;
            bool activated_by_input_result;

            // These two time values enclose the first detected state event 
            // while integrating
            double t_max_se, t_min_se;
            // Stores the state at the lower bound of the interval 
            // where an state event has been detected
            sct_core::ct_state x_min_se, x_max_se;


            // We need to store the events detected in the 
            // previous iteration because inputs may change 
            // between iterations, and ode_system::is_event would not
            // produce the same result the moment in the iteration 
            // where the event is detected and in the one when 
            // outputs are generated 
            // std::unordered_map<int, bool>  events;
            bool events;

            // List of events to keep sensitivity on inputs
            sc_core::sc_event_or_list input_event_sensitivity_list;
            // 
            bool is_first_activation;
            // Functions commonly used : differential equations, threshold detection and threshold observer
            std::function<void(const sct_core::ct_state &x, sct_core::ct_state &dxdt, double t)> diff_equations_func;
            std::function<bool(const sct_core::ct_state &x, double t)> is_event_func;
            std::function<bool(const sct_core::ct_state &, const double)> state_event_observer;
            // Solver
            sct_kernel::numerical_solver *solver;
            // To collect some statistics
            int backtrack_count;
            double estimated_time_between_inputs;
            sc_core::sc_time last_input_event_time;
            std::ofstream *statistics_file;

            bool activated_by_input; 

            /////////////////////////////////////////////////////////
            // PRIVATE METHODS AND UTILITY FUNCTIONS
            /////////////////////////////////////////////////////////
            synchronization_layer();
            void initialize_local_time(double max_timestep);
            void initialize_solver();
            void extract_equations_from_system();
            void store_system_model(ode_system *model_ptr);
            void create_synchronization_process();
            void handle_reactivation();
            bool had_advanced_real_time();

            void verify_causality();
            void backtrack();
            void create_backup();
            void schedule_reactivation();
            void calculate_solutions();
            bool calculate_delta_step_dynamics(double t_start);
            double get_tentative_end_time();
            double get_step();
            double integrate_equations(double t_start, double t_end);
            // Advance local time to interval end time or crossing time
            void advance_time(double t_end);
            // Returns threshold crossing time if any in (t_start, t_end])
            double handle_state_events(double t_start, double t_end);
            bool locate_state_event_at_start_of_interval(const double t);
            void catch_up();
            void watch_state_events( const sct_core::ct_state &x , const double t );
            bool is_delta_cycle();
            void get_global_time();
            void clear_event_flags();
            bool locate_state_event(const double t_start, double &t_end, sct_core::ct_state &state_at_event, const sct_core::ct_state &state_end);
            void initialize_statistics();
            void collect_input_event_statistics();

            bool first_registering;


            ////////////////////////////////////////////////////////////////////
            // PARALLELIZATION
            ////////////////////////////////////////////////////////////////////
            void parallel_registering_process();
            void add_to_parallel_runnable_processes(); 
            sc_core::sc_event finished_execution_ev;



    };
}
#endif