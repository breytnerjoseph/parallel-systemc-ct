#ifndef NEURON_H
#define NEURON_H

#include <systemc>
#include "ct_module.h"
#include "synapse.h"

// Length of integration interval
#define DELTA_T 2 // seconds

#define SAMPLING_PERIOD 0.05 //seconds

class Neuron : public sct_core::ct_module{
    public:
        //////////////////////////////////////////////////////////////
        // Output events 
        //////////////////////////////////////////////////////////////
        synapse_out spike_out;
        //////////////////////////////////////////////////////////////
        // Input events 
        //////////////////////////////////////////////////////////////
        synapse_in excitatory_in;
        synapse_in inhibitory_in;


        //////////////////////////////////////////////////////////////
        // Public methods 
        //////////////////////////////////////////////////////////////
        Neuron(sc_core::sc_module_name name,
            bool use_adaptive = true, bool avoid_rollback = true, 
            double sync_step = DELTA_T, bool parallel_execution = true
            ) :
            sct_core::ct_module(name, use_adaptive, avoid_rollback, parallel_execution),
            spike_out("spike_out"),
            next_sampling_time(0, sc_core::SC_SEC),
            avoid_rollback(avoid_rollback),
            use_adaptive(use_adaptive), 
            sync_step(sync_step)
        { 
            Vr = 49*1e-3; // 49 mV
            V_reset = -60*1e-3; // -60 mV
            threshold = -50*1e-3; // -50 mV
            tau = 20*1e-3; // 20 ms
            tau_e = 15*1e-3; // 5 ms
            tau_i = 10*1e-3; // 10 ms
            w_e = 1.62*1e-3; // 1.62 mV
            w_i = -9*1e-3; // -9 mV

            inhibitory_synapse_ptr = excitatory_synapse_ptr = spike_synapse_ptr = NULL;
            inhibitory_connected = excitatory_connected = spike_connected = false;

            // type = excitatory_type;
        }

        void set_sync_parameters(){  
            sync_step = sync_step <= 0 ? DELTA_T : sync_step;
            set_max_timestep(sync_step);
            use_adaptive_sync(use_adaptive);
            avoid_rollbacks(avoid_rollback);
        }

        void set_initial_conditions(){
            // x.resize(1);
            // x[0] = 0.0;

            x.resize(3);
            x[0] = -40*1e-3; // Membrane potential
            x[1] = 0; // Excitatory synapses
            x[2] = 0; // Inhibitory synapses

        }

        void get_derivatives(bool use_input_checkpoints,
            const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t){
             // Set the mode of the inputs manager to either current_values or checkpoint
            // std::cout << "Start get_derivatives "<< t << " and " << sc_core::sc_time_stamp() << std::endl;

            ode_system::inputs.use_checkpoints(use_input_checkpoints);


            // 
            double V = x[0];
            double ge = x[1];
            double gi = x[2];

            dxdt[0] = (-(V + Vr) + ge + gi) / tau;
            dxdt[1] = -ge / tau_e;
            dxdt[2] = -gi / tau_i;

            // std::cout << "End get derivatives "<< t << " and " << sc_core::sc_time_stamp() << std::endl;
        }

    //    std::unordered_map<int, bool> is_event(const sct_core::ct_state &x, double t = 0) {
    //         std::unordered_map<int, bool> events;            
    //         // The switch closed and up threshold reached
    //         events[threshold_event] = x[0] >= threshold;

    //         return events;
    //     }

        bool is_event(const sct_core::ct_state &x, double t = 0) {
            bool events;            
            // The switch closed and up threshold reached
            
            events = is_threshold_event(x); 

            return events;
        }

        bool is_threshold_event(const sct_core::ct_state &x)  {
            return x[0] >= threshold;
        }

        // void generate_outputs(bool state_event_located, 
        //     std::unordered_map<int, bool> events
        // ) {
        //     // std::cout << "state event located " << state_event_located << std::endl;
        //     if (state_event_located) {
        //         // If it is an up energy event
        //         if (events[threshold_event]) {
                    
        //             // Reset state
        //             x[0] = V_reset;
                    
        //             // Write outputs 
        //             for (int i = 0; i < spike_out.size(); i++) {
        //                 spike_out[i]->increment();
        //             }
        //         }

        //     }
        // }


        void generate_outputs(bool state_event_located, 
            bool events
        ) {
            // std::cout << "state event located " << state_event_located << std::endl;
            if (state_event_located) {
                // If it is an up energy event
                if (is_threshold_event(x)) {
                    
                    // Reset state
                    x[0] = V_reset;
                    
                    // Write outputs 
                    for (int i = 0; i < spike_out.size(); i++) {
                        spike_out[i]->increment();
                    }
                }

            }
        }

        bool execute_updates() {
            bool updated = false;
            // New excitatory inputs 
            if (excitatory_in.event()) {
                x[1] += w_e*excitatory_in.read();
                updated = true;
            }
            // New inhibitory inputs
            if (inhibitory_in.event()) {
                x[2] += w_i*inhibitory_in.read();
                updated = true;
            }

            return updated;
        }
        
        /********************************************
         * Methods unrelated to ct_model but useful 
         * for modeling the SNN
         * *******************************************/

        synapse *get_excitatory_synapse() {
            return excitatory_synapse_ptr;
        }

        synapse *get_inhibitory_synapse() {
            return inhibitory_synapse_ptr;
        }
        
        void set_excitatory_synapse(synapse *ptr) {
            excitatory_synapse_ptr = ptr;
        }

        void set_inhibitory_synapse(synapse *ptr) {
            inhibitory_synapse_ptr = ptr;
        }

        void set_spike_synapse(synapse *ptr) {
            spike_synapse_ptr = ptr;
        }

        synapse *get_spike_synapse() {
            return spike_synapse_ptr;
        }

        bool is_excitatory_connected() {
            return excitatory_connected;
        }

        bool is_inhibitory_connected() {
            return inhibitory_connected;
        }

        bool is_spike_connected() {
            return spike_connected;
        }


        void set_excitatory_connected(bool connected) {
            excitatory_connected = connected;
        } 

        void set_inhibitory_connected(bool connected) {
            inhibitory_connected = connected;
        }

        void set_spike_connected(bool connected) {
            spike_connected = connected;
        }

        enum synapse_type {excitatory_type, inhibitory_type};

        void set_type(synapse_type t) {
            type = t;
        }

        synapse_type get_type() {
            return type;
        } 


    private:
        // System parameters
        double Vr;
        double V_reset;
        double threshold;
        double tau;
        double tau_e;
        double tau_i;
        double w_e;
        double w_i;


        // enum event_ids {threshold_event};

        synapse_type type;

        sc_core::sc_time next_sampling_time;

        synapse *inhibitory_synapse_ptr, *excitatory_synapse_ptr,  *spike_synapse_ptr;
        bool inhibitory_connected, excitatory_connected, spike_connected;

        // Sync. parameters
        bool avoid_rollback, use_adaptive, parallel_execution; 
        double sync_step;

};

#endif