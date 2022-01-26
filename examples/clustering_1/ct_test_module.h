#ifndef CIRCUIT_SC_MODULE_H
#define CIRCUIT_SC_MODULE_H

#include <systemc>
#include "ct_module.h"

// Length of integration interval
#define DELTA_T 2 // seconds

#define SAMPLING_PERIOD 0.05 //seconds

class CtTestModule : public sct_core::ct_module{
    public:
        //////////////////////////////////////////////////////////////
        // Input events 
        //////////////////////////////////////////////////////////////
        sc_core::sc_in<bool> switch_in; 

        //////////////////////////////////////////////////////////////
        // Public methods 
        //////////////////////////////////////////////////////////////
        CtTestModule(sc_core::sc_module_name name = sc_core::sc_gen_unique_name("module"),
            bool use_adaptive = true, bool avoid_rollback = false, 
            double sync_step = DELTA_T, bool parallel_execution = true
            ) :
            sct_core::ct_module(name, use_adaptive, avoid_rollback, parallel_execution),
            switch_in("switch_in"), 
            next_sampling_time(0, sc_core::SC_SEC),
            avoid_rollback(avoid_rollback),
            use_adaptive(use_adaptive), 
            sync_step(sync_step), 
            on_threshold_distribution(on_threshold_mean, on_threshold_standard_dev),
            off_threshold_distribution(off_threshold_mean, 
            off_threshold_standard_dev)
        { 
            // Distribution generator seeds
            auto name_str = std::string(name);
            std::seed_seq seed (name_str.begin(), name_str.end());
            generator.seed(seed);

            on_threshold = on_threshold_distribution(generator);
            off_threshold = off_threshold_distribution(generator);
            last_detected_event = off_th_event;
        }

        void set_sync_parameters(){  
            sync_step = sync_step <= 0 ? DELTA_T : sync_step;
            
            set_max_timestep(sync_step);
            use_adaptive_sync(use_adaptive);
            avoid_rollbacks(avoid_rollback);
        }

        void set_initial_conditions(){
            x.resize(1);
            x[0] = 0;
            // x[1] = 2.0 / 3.0;
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

                // dxdt[0] = 9*x[0] + 24*x[1];// + 5*cos(t) - (1.0/3.0)*sin(t);
                // dxdt[1] = -24*x[0] - 51*x[1];// - 9*cos(t) + (1.0/3.0)*sin(t);

                // double a = 9*x[0] + 24*x[1] + 5*cos(t) - (1.0/3.0)*sin(t); 
                // double b = -24*x[0] - 51*x[1] - 9*cos(t) + (1.0/3.0)*sin(t);

                // dxdt[0] *= 2;
                // dxdt[0] -= a;
                // dxdt[1] *= 2;
                // dxdt[1] -= b;

            }
            else {

                dxdt[0] = -1/(R2*C)*x[0];
                // dxdt[0] = 9*x[0] + 24*x[1] + switch_offset; // + 5*cos(t) - (1.0/3.0)*sin(t) + switch_offset;
                // dxdt[1] = -24*x[0] - 51*x[1] + switch_offset; // - 9*cos(t) + (1.0/3.0)*sin(t) + switch_offset; 

                // double a = 9*x[0] + 24*x[1] + 5*cos(t) - (1.0/3.0)*sin(t) + switch_offset;
                // double b = -24*x[0] - 51*x[1] - 9*cos(t) + (1.0/3.0)*sin(t) + switch_offset; 

                // dxdt[0] *= 2;
                // dxdt[0] -= a;
                // dxdt[1] *= 2;
                // dxdt[1] -= b;

            }




        }

        bool execute_updates() {
            // Each CT module has different thesholds to make the example general
            // These thresholds change from time to time to avoid one of 
            // the modules (the one with the closest threshold to its state) 
            // dictate the time at which all CT modules synchronize 
            if (should_update_thresholds()) {
                // Threshold follow the same probability distribution 
                // to simulate similar (but not necessrily equal) time constants
                // across modules
                on_threshold = on_threshold_distribution(generator);
                off_threshold = off_threshold_distribution(generator);
                std::cout << "Ex update " << std::endl;
                return true;
            }

            return false;
        }

        bool should_update_thresholds() {
            // When inputs change, update thersholds 1/3 of the time
            return inputs.have_inputs_changed() && ((rand() % 3) < 1);
        }


        bool is_event(const sct_core::ct_state &x, double t = 0) {
            bool events = is_on_th_event(x) || is_off_th_event(x);

            return events;
        }

        void generate_outputs(bool state_event_located, 
            bool is_event
        ) {
            if (state_event_located) {
                x[1] = 0;
                x[2] = 0;

                // To avoid detecting event twice
                if (is_on_th_event(x)) {
                    last_detected_event = on_th_event;
                }
                else {
                    last_detected_event = off_th_event;
                }
            }
        }

        bool is_on_th_event(const sct_core::ct_state &x) { 
            return  (last_detected_event == off_th_event && switch_in.read() && x[0] <= on_threshold);
        }

        bool is_off_th_event(const sct_core::ct_state &x) {
            return (last_detected_event == on_th_event && !switch_in.read() && x[0] <= off_threshold);
        }

    private:
        // System parameters
        double R1 = 0.5;
        double R2 = 0.5;
        double C = 1;
        double Vi = 1.0;
        double switch_offset = -2;
        double on_threshold;
        double off_threshold;

        double on_threshold_mean = -1.2;
        double on_threshold_standard_dev = 0.05;

        double off_threshold_mean = 0.0;
        double off_threshold_standard_dev = 0.1;

        std::default_random_engine generator;
        std::normal_distribution<double> on_threshold_distribution;
        std::normal_distribution<double> off_threshold_distribution;

        

        enum event_ids {on_th_event, off_th_event};

        event_ids last_detected_event;

        sc_core::sc_time next_sampling_time;

        // Sync. parameters
        bool avoid_rollback, use_adaptive, parallel_execution; 
        double sync_step;

};

#endif