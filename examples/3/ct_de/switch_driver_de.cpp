#include "switch_driver_de.h"

SwitchDriverDE::SwitchDriverDE(sc_core::sc_module_name name){
    SC_METHOD(up_ctrl_thread);
    dont_initialize();
    sensitive << up_in << down_in;
    
    switch_out.initialize(true);
    last_up_event = false;

    count = 0;
}

void SwitchDriverDE::control_switch(bool sw_state){
    switch_out.write(sw_state);
}

void SwitchDriverDE::up_ctrl_thread(){

    auto t1 = std::chrono::high_resolution_clock::now();

    // up event
    if (up_in.read() != last_up_event) {
        control_switch(false);
        last_up_event = up_in.read();
    }
    // down event
    else {
        control_switch(true);
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
    exec_time += duration * 1e-6;
    
}
