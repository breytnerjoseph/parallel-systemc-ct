#ifndef SWITCH_DRIVER_DE_H
#define SWITCH_DRIVER_DE_H

#include<systemc>
#include<chrono>
#include "synapse.h"

// Module that generates
// the switch control signal
SC_MODULE(SwitchDriverDE){

    // Outputs
    synapse_out excitatory_out;
    synapse_out inhibitory_out;
    
    
    SC_CTOR(SwitchDriverDE);


    void up_ctrl_thread();
    
    private:

};

#endif