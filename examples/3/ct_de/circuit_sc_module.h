#ifndef CIRCUIT_SC_MODULE_H
#define CIRCUIT_SC_MODULE_H

#include <systemc>
#include "ct_module.h"

// Length of integration interval
#define DELTA_T 2 // seconds

#define SAMPLING_PERIOD 0.05 //seconds

class CircuitScModule : public sct_core::ct_module{
    public:
        //////////////////////////////////////////////////////////////
        // To save traces with the usual SystemC AMS mechanism.
        // We need to provide a mechanism that does not depend on 
        // signals to record the CT state, because the CT module 
        // communicates with the external world through events 
        // that do not necessarily contain all the information we might
        // want to record about the internals of the CT module
        //////////////////////////////////////////////////////////////
        // Capacitor's voltage
        sc_core::sc_out<double> v_out;

        //////////////////////////////////////////////////////////////
        // Output events 
        //////////////////////////////////////////////////////////////
        sc_core::sc_out<bool> up_out;
        sc_core::sc_out<bool> down_out;

        //////////////////////////////////////////////////////////////
        // Input events 
        //////////////////////////////////////////////////////////////
        sc_core::sc_in<bool> switch_in; 


        //////////////////////////////////////////////////////////////
        // Public methods 
        //////////////////////////////////////////////////////////////
        CircuitScModule(sc_core::sc_module_name name,
            bool use_adaptive = true, bool avoid_rollback = true, 
            double sync_step = DELTA_T, bool parallel_execution = true
            ) :
            sct_core::ct_module(name, use_adaptive, avoid_rollback, parallel_execution),
            switch_in("switch_in"), 
            next_sampling_time(0, sc_core::SC_SEC),
            avoid_rollback(avoid_rollback),
            use_adaptive(use_adaptive), 
            sync_step(sync_step)
        { 

        }

        void set_sync_parameters(){  
            sync_step = sync_step <= 0 ? DELTA_T : sync_step;
            
            set_max_timestep(sync_step);
            use_adaptive_sync(use_adaptive);
            avoid_rollbacks(avoid_rollback);
        }

        void set_initial_conditions(){
            x.resize(1);
            x[0] = 0.0;
            // x[1] = 0.0;
            // x[2] = 0.0;

        }

        void get_derivatives(bool use_input_checkpoints,
            const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t){
             // Set the mode of the inputs manager to either current_values or checkpoint
            ode_system::inputs.use_checkpoints(use_input_checkpoints);
            // Get the value from the stop_in port
            bool switch_closed = ode_system::inputs[switch_in];

            if (switch_closed) {
                dxdt[0] = Vi/(C*R1) - ((R1+R2)/(C*R1*R2))*x[0];
            }
            else {
                dxdt[0] = -1/(R2*C)*x[0];
            }

        }


       bool is_event(const sct_core::ct_state &x, double t = 0) {
            // std::unordered_map<int, bool> events;            
            // // The switch closed and up threshold reached
            // events[up_event] = (switch_in.read() && x[0] >= max_threshold);

            // // The switch is open and the down threshold reached
            // events[down_event] = (!switch_in.read() && x[0] <= min_threshold);

            bool events = is_up_event(x) || is_down_event(x);
            
            return events;
        }

        void generate_outputs(bool state_event_located, 
            bool is_event
        ) {
            if (state_event_located) {
                // If it is an up energy event
                if (is_up_event(x)) {
                    up_out.write(!up_out.read());
                }

                if (is_down_event(x)) {
                    down_out.write(!down_out.read());
                }

            }
            v_out.write(x[0]);
        }

        bool is_up_event(const sct_core::ct_state &x) {
            return  (switch_in.read() && x[0] >= max_threshold);
        }

        bool is_down_event(const sct_core::ct_state &x) {
            return (!switch_in.read() && x[0] <= min_threshold);
        }

    private:
        // System parameters
        double R1 = 0.5;
        double R2 = 0.5;
        double C = 1;
        double Vi = 1.0;
        double max_threshold = 0.49;
        double min_threshold = 0.05;

        enum event_ids {up_event, down_event};

        sc_core::sc_time next_sampling_time;

        // Sync. parameters
        bool avoid_rollback, use_adaptive, parallel_execution; 
        double sync_step;

};

#endif