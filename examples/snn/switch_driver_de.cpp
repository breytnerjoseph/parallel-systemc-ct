#include "switch_driver_de.h"

SwitchDriverDE::SwitchDriverDE(sc_core::sc_module_name name) : 
    excitatory_out("ex_out"), inhibitory_out("inh_out")
{
    SC_THREAD(up_ctrl_thread);
    // dont_initialize();
}


void SwitchDriverDE::up_ctrl_thread(){
    while (true) {
        // excitatory_out->increment();
        wait(5, sc_core::SC_SEC);
    } 
}
