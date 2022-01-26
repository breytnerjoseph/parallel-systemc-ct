#ifndef SWITCH_DRIVER_DE_H
#define SWITCH_DRIVER_DE_H

#include<systemc>
#include<chrono>

// Module that generates
// the switch control signal
SC_MODULE(SwitchDriverDE){
    // Inputs
    sc_core::sc_in<bool> up_in;
    sc_core::sc_in<bool> down_in;

    // Outputs
    sc_core::sc_out<bool> switch_out;
    
    SC_CTOR(SwitchDriverDE);

    void control_switch(bool sw_state);
    void up_ctrl_thread();
    void down_ctrl_thread();

    void print_exec_time() {
        std::cout << "Driver's execution time " << exec_time << " seconds" << std::endl;
    }

    private:
        // Needed until we define a proper communication channel
        bool last_up_event; 

        int count;

        double exec_time;
};

#endif