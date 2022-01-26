#include "transmission_control_unit.h"


TransmissionControlUnit ::TransmissionControlUnit(sc_core::sc_module_name name)
    : state_selector("state_selector"), gear_state("gear_state"), 
        direction_sig("direction_sig"), update_gear_evq("update_gear_evq")
{
    state_selector.update_in(update_in);
    state_selector.threshold_in(threshold_in);
    
    state_selector.update_out(update_gear_evq);
    state_selector.direction_out(direction_sig);

    gear_state.update_in(update_gear_evq);  
    gear_state.direction_in(direction_sig);

    gear_state.gear_out(gear_out);
    gear_state.gear_ratio_out(gear_ratio_out);
}
