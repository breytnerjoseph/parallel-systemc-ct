#ifndef ODE_SYSTEM_H
#define ODE_SYSTEM_H

#include <systemc>

#include "inputs_manager.h"
#include "ct_state.h"
#include <map>
#include <list>
#include <fstream>
#include <string>

// TBB
#include "tbb/tbb_allocator.h"
#include "tbb/scalable_allocator.h"

namespace sct_kernel {
    class synchronization_layer;

    class ode_system {
        public:
            ode_system();

            /////////////////////////////////////////////
            // VIRTUAL METHODS TO BE SPECIFIED BY USER 
            /////////////////////////////////////////////

            // Initial conditions
            virtual void set_initial_conditions() = 0;

            // Equations
            virtual void get_derivatives( bool use_inputs_checkpoint,
                const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t 
            ) = 0;

            // Threshold crossing condition.
            // Conceptually, it depends on the inputs. But they are not
            // passed because only he current values are needed (not the 
            // checkpoint and they are available through the ports)
            // It returns an unordered map of pairs (int event_identifier, bool occurred) 
            // signaling the conditions that have been met. 
            // Identifiers belong to the set
            // of integers and their meaning depend on the model.  
            // virtual std::unordered_map<int, bool>  is_event(const sct_core::ct_state &x, double t = 0);
            virtual bool  is_event(const sct_core::ct_state &x, double t = 0);

            // Output function
            // virtual void generate_outputs(bool state_event_located,
            //     std::unordered_map<int, bool>  events) = 0;
            virtual void generate_outputs(bool state_event_located, 
                bool events) = 0;

            // Calculates user-defined instantaneous state updates
            // based on changes at the inputs 
            // (current inputs only, no need to pass them as args)
            // (Muxes located exactly after integrators, for example)
            // Returns true if an instantaneous update has taken place
            // and false otherwise
            virtual bool execute_updates();

            virtual sc_core::sc_time get_next_predictable_event_time();

            /////////////////////////////////////////////
            // GENERAL METHODS
            /////////////////////////////////////////////

            void create_checkpoint();

            void backup_state();

            double restore_state(double t);

            sct_core::ct_state &get_state();

            bool set_state(sct_core::ct_state x);

            const sct_core::ct_state &get_backup_state();

            // The synch. layer is agnostic about the existence of the inputs 
            // manager. This way, MoCs are not obliged to depend on an 
            // input manager. But they are obliged to provide the methods 
            // on which the synchronization layer depends. 
            bool have_inputs_changed();

            // Tracing
            virtual const sct_core::ct_state &map_state_to_trace(double t, const sct_core::ct_state &x);
            void write_trace(double t, const sct_core::ct_state &x);
            void create_trace_file(std::string file_name);
            void restore_trace(double t);

            ////////////////////////////////////////////////////
            // METHODS USED BY THE PARALLELIZATION ALGORITHM
            ////////////////////////////////////////////////////
            inputs_manager &get_inputs_manager();
            // Execute in a new interval.
            // This is implemented by the ct_module, 
            // which has access to the synchronization layer object
            virtual void execute() = 0;

            // Add to set of parallel runnable processes in the current delta cycle
            virtual void add_to_parallel_runnable_processes() = 0;
            virtual void notify_finished_exec() = 0;

            // Return the local time of the system during simulation
            // Implemented by the ct_module which has access to the 
            // synchronization layer object
            virtual const sc_core::sc_time &get_local_time() const = 0;

            virtual const char *get_proc_name() const = 0;

            virtual bool should_generate_outputs() = 0;
            virtual void generate_outputs() = 0;

            std::pair<double, sct_core::ct_state> trace_list_node;

            virtual synchronization_layer *get_sync_layer() = 0;

            // bool get_state_at(double &t, sct_core::ct_state &x);

        protected:
            sct_core::ct_state x, x_backup;
            inputs_manager inputs;
            // std::map<double, sct_core::ct_state> trace_map;
            typedef std::pair<double, sct_core::ct_state> trace_node;

            // #ifdef USE_TBB
            //     std::list<trace_node, tbb::tbb_allocator<trace_node> > trace;
            // #else 
                std::list<trace_node> trace;
            // #endif

    };

}

#endif