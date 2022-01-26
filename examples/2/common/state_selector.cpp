#include "state_selector.h"


StateSelector::StateSelector(sc_core::sc_module_name name){
    state = steady;

    activations_count = 0;

    SC_METHOD(generate_output);
    sensitive << output_timer;
    dont_initialize();

    SC_METHOD(update_state);
    sensitive << update_in;
    dont_initialize();
}
        
void StateSelector::update_state(){
    activations_count++;

    switch (threshold_in.read()) {
        case speed_less_th_down:
            if (state == steady) {
                state = downshifting;
                // Generate output within 80ms
                output_timer.notify(80, sc_core::SC_MS);
            }
            break;
        
        case speed_greater_th_down:
            if (state == downshifting) {
                state = steady;
                // Cancel ouput generation
                output_timer.cancel();
            }
            break;
        
        case speed_greater_th_up:
            if (state == steady) {
                state = upshifting;
                // Generate output within 80ms
                output_timer.notify(80, sc_core::SC_MS);
            }
            break;

        case speed_less_th_up:
            if (state == upshifting) {
                state = steady;
                // Cancel ouput generation
                output_timer.cancel();
            }
            break;

        default:
            break;
    }
}

void StateSelector::generate_output(){
    #ifdef DEBUG 
        td::cout << "STATE-SELECTOR: Generating output at " << sc_core::sc_time_stamp().to_seconds() << " state " << state ;
    #endif
    // Write direction
    if (state == downshifting){
        direction_out.write(false);
    }
    else if (state == upshifting) {
        direction_out.write(true);
    }
    // Notify update
    update_out->notify(sc_core::SC_ZERO_TIME);

    state = steady;
}

void StateSelector::end_of_simulation(){
    #ifdef DEBUG 
        std::cout << "Activated StateSelector " << activations_count << " times " << std::endl;
    #endif
}