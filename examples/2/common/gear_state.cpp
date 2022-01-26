#include "gear_state.h"

// Mapping gear values to gear ratios
const double GearState::gear_ratios[4] = {2.34, 1.450, 1.000, 0.68};

GearState::GearState(sc_core::sc_module_name name){
    state = first;

    activations_count = 0;

    SC_METHOD(update_gear);
    sensitive << update_in;
    dont_initialize();

     gear_out.initialize((double) state);
     gear_ratio_out.initialize(GearState::gear_ratios[state-1]);

}

void GearState::update_gear(){
    activations_count++;
    if (direction_in.read()) {
        increase_gear();
    }
    else {
        decrease_gear();
    }
    generate_output();
}

void GearState::increase_gear(){
    switch(state){
        case first:
            state = second;
            break;
        case second:
            state = third;
            break;
        case third:
            state = fourth;
            break;
        default:
            break;
    }
}

void GearState::decrease_gear(){
    switch(state){
        case second:
            state = first;
            break;
        case third:
            state = second;
            break;
        case fourth:
            state = third;
            break;
        default:
            break;
    }
}

void GearState::generate_output(){
    #ifdef DEBUG
        std::cout << "Changing gear at " << sc_core::sc_time_stamp().to_seconds() << std::endl;
    #endif
    gear_out.write((double) state);
    gear_ratio_out.write(gear_ratios[state-1]);
}


void GearState::end_of_simulation(){
    #ifdef DEBUG
        std::cout << "Activated GearState " << activations_count << " times " << std::endl;
    #endif
}