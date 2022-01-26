#include<systemc>
#include<iostream> 
#include <string>
#include <chrono>
#include <stdlib.h>  

#include "switch_driver_de.h"
#include "neuron.h"

#include "sync_conf_utility.h"
#include "synapse.h"


// Pass as first parameter y to use the adaptive sync. strategy,
// y as second parameter to avoid rollbacks (use time of the next
// event), and a real number as the third parameter to set the timestep
int sc_main(int argc, char* argv[]){

    auto t1 = std::chrono::high_resolution_clock::now();

    sc_core::sc_time simulated_time(0.05, sc_core::SC_SEC);
    bool use_adaptive = true;
    bool avoid_rollback = false;
    double timestep = 0;
    bool parallel_execution = false;

    get_sync_conf(argc, argv, use_adaptive, avoid_rollback, timestep, parallel_execution);


    int max_neurons = 4000;
    std::vector<Neuron *> neurons;
    int excitatory_index = 3200;
    
    // CREATE SET OF NEURONS
    for (int i = 0; i < max_neurons; i++) {
        // Create neuron
        Neuron *neuron_ptr = new Neuron( (std::string("neuron") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);

        // Create synapse signals for the neuron
        synapse *exc_synapse_ptr = new synapse;
        synapse *inh_synapse_ptr = new synapse;

        // Save excitatory synapse for later binding
        neuron_ptr->set_excitatory_synapse(exc_synapse_ptr);

        // Save inhibitory synapse or later binding
        neuron_ptr->set_inhibitory_synapse(inh_synapse_ptr);

        // Set type
        if (i < excitatory_index) {
            neuron_ptr->set_type(Neuron::excitatory_type);
        }
        else {
            neuron_ptr->set_type(Neuron::inhibitory_type);
        }

        neurons.push_back(neuron_ptr);
    }

    // Mock driver 
    synapse mock_exc_synapse, mock_inh_synapse;
    SwitchDriverDE driver("driver");
    driver.excitatory_out(mock_exc_synapse);
    driver.inhibitory_out(mock_inh_synapse);

    // CONNECT NEURONS
    for (auto neuron_i_ptr : neurons) {

        for (auto neuron_j_ptr: neurons) {
            
            if (neuron_i_ptr != neuron_j_ptr) {

                // Connect neurons with a probability of 2 %
                bool connect_neurons = (rand() % 100) < 2;
                
                if (connect_neurons) {
                    synapse *synapse_ptr;
                
                    // Connect according to synapse type
                    if (neuron_i_ptr->get_type() == Neuron::excitatory_type) {
                        synapse_ptr = neuron_j_ptr->get_excitatory_synapse();
                        neuron_i_ptr->spike_out.bind(*synapse_ptr);
                        
                        // If not yet bound
                        if (!neuron_j_ptr->is_excitatory_connected()) {
                            neuron_j_ptr->excitatory_in.bind(*synapse_ptr);
                            neuron_j_ptr->set_excitatory_connected(true);
                        }
                    }
                    else if (neuron_i_ptr->get_type() == Neuron::inhibitory_type) {
                        synapse_ptr = neuron_j_ptr->get_inhibitory_synapse();
                        neuron_i_ptr->spike_out.bind(*synapse_ptr);
                        
                        // If not yet bound 
                        if (!neuron_j_ptr->is_inhibitory_connected()) {
                            neuron_j_ptr->inhibitory_in.bind(*synapse_ptr);
                            neuron_j_ptr->set_inhibitory_connected(true);
                        }
                    }
                    
                    neuron_i_ptr->set_spike_connected(true);
                }                
            }
        }
    }

    for (auto neuron_ptr : neurons) {

        if (!neuron_ptr->is_spike_connected()) {
            neuron_ptr->spike_out.bind(*(new synapse));
            neuron_ptr->set_spike_connected(true);
        } 

        if (!neuron_ptr->is_excitatory_connected()) {
            neuron_ptr->excitatory_in.bind(mock_exc_synapse);
            neuron_ptr->set_excitatory_connected(true);
        }

        if (!neuron_ptr->is_inhibitory_connected()) {
            neuron_ptr->inhibitory_in.bind(mock_inh_synapse);
            neuron_ptr->set_inhibitory_connected(true);
        }

    }


    // SwitchDriverDE switch_driver1("driver1");
    // SwitchDriverDE switch_driver2("driver2");

    // int i = 0;
    // Neuron neuron( (std::string("neuron") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);

    // synapse ex_synapse;
    // synapse spike;

    // switch_driver1.excitatory_out(ex_synapse);
    // switch_driver2.excitatory_out(ex_synapse);
    
    // neuron.excitatory_in(ex_synapse);
    // neuron.spike_out(spike);

    // int nb_circuits = 1;

    // for (int i = 0; i < nb_circuits; i++) {
        
    //     CircuitScModule *circuit_ptr = new CircuitScModule( (std::string("circuit") +  std::to_string(i)).c_str(), use_adaptive, avoid_rollback, timestep, parallel_execution);

    //     // sc_core::sc_signal<double> *v_sg_ptr = new sc_core::sc_signal<double>;
    //     synapse *v_sg_ptr = new synapse;
    //     sc_core::sc_signal<bool> *up_sg_ptr = new sc_core::sc_signal<bool>;
    //     sc_core::sc_signal<bool> *down_sg_ptr = new sc_core::sc_signal<bool>;
        
    //     circuit_ptr->v_out.bind(*v_sg_ptr);
    //     circuit_ptr->switch_in.bind(switch_sg_1);
    //     circuit_ptr->up_out.bind(*up_sg_ptr);
    //     circuit_ptr->down_out.bind(*down_sg_ptr);

    //     // Connect driver circuit
    //     if (i == 0) {
    //         switch_driver1.up_in.bind(*up_sg_ptr);
    //         switch_driver1.down_in.bind(*down_sg_ptr);
    //     }

    // }

    // switch_driver1.switch_out.bind(switch_sg_1);


    // Start simulation for a given time
    sc_core::sc_start(simulated_time);

    // Finish simulation
    sc_core::sc_stop();
 
    auto t2 = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();


    print_sync_stats(use_adaptive, avoid_rollback, timestep, simulated_time.to_seconds(), duration * 1e-06);

    // DELETE
    for (int i = 0; i < max_neurons; i++) {
        delete neurons[i]->get_excitatory_synapse();
        delete neurons[i]->get_inhibitory_synapse();
        delete neurons[i];
    }

    return 0;
}