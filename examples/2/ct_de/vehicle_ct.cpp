#include "vehicle_ct.h"


VehicleCt::VehicleCt(sc_core::sc_module_name name,
    bool use_adaptive, bool avoid_rollback, 
    double sync_step, bool parallel_execution)
    : sct_core::ct_module(name, use_adaptive, avoid_rollback, parallel_execution), 
    te_calculator(), th_calc(),
    use_adaptive(use_adaptive), 
    avoid_rollback(avoid_rollback),
    sync_step(sync_step)
{
    last_written_threshold_out = speed_greater_th_down;
    threshold_out.initialize((ShiftingThresholds) last_written_threshold_out);

    #ifdef WRITE_INTERNAL_DATA_TO_FILE 
        myfile = new std::ofstream();
        myfile->open("internal.txt");
    #endif
}

// OPTIONAL METHODS
void VehicleCt::set_sync_parameters(){
    sync_step = sync_step <= 0 ? DELTA_T : sync_step;
    
    set_max_timestep(sync_step);
    use_adaptive_sync(use_adaptive);
    avoid_rollbacks(avoid_rollback);
}


// MANDATORY METHODS
void VehicleCt::set_initial_conditions(){
    x.resize(3);
    x[0] = 1000.0;
    x[1] = 0.0;
    x[2] = 0.0;
}

void VehicleCt::get_derivatives(bool use_input_checkpoints,
            const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t)
{
    // Set the mode of the inputs manager to either current_values or checkpoint
    inputs.use_checkpoints(use_input_checkpoints);

    double Ne = x[0];
    double v = x[1];
    double Pb = x[2];

    double throttle = get_throttle(0.0);
    
    double Rtr = inputs[gear_ratio_in];
    
    double Te = te_calculator.get_torque(Ne, throttle);
    
    double Ktc = constants_calculator.get_capacity_factor(v, Ne, Rtr);
    
    double Ctr = constants_calculator.get_torque_ratio(v, Ne, Rtr);
    


    double Ti = pow((Ne/Ktc),2.0);
    // Ne equations    
    dxdt[0] = (1.0 / Iei) * (Te - Ti);

    // Vehicle speed equation
    dxdt[1] = (1.0 / m) * ( (1.0 / r) * (Rtr * Rf * Ctr * Ti - Kb * Pb) 
        - ( 1.0 / 2.0 ) * rhoACd * pow(v + vw, 2.0) - Cr * m * g * cos(theta) 
        + m * g * sin(theta) ); 
    
    // Braking system equations
    dxdt[2] = (-1.0 / taub ) * (Pb - (150.0 * Kc * ub));

}

// Threshold crossing condition.
// std::unordered_map<int, bool> VehicleCt::is_event(const sct_core::ct_state &x, double t) {
//     std::unordered_map<int, bool>  events;
//     double speed = x[1];
//     double throttle_percentage = get_throttle(0.0)*100;
//     double up_th = th_calc.getUpperThreshold(gear_in.read(), throttle_percentage);
//     double down_th = th_calc.getLowerThreshold(gear_in.read(), throttle_percentage);

//     int threshold_crossed = check_thresholds(up_th, down_th, speed);

//     #ifdef WRITE_INTERNAL_DATA_TO_FILE 
//         write_internal_data(up_th, down_th);
//     #endif

//     // std::cout << "** IS_EVENT called at " << t << " speed: " << speed << std::endl;
//     // std::cout << " speed = " << speed << std::endl;

//     if(threshold_crossed != 0){
//         // std::cout << "DETECTED EVENT AT " << t <<  "EVENT: " << threshold_crossed << std::endl;
//         events[threshold_crossed] = true;
//     }
    
//     return events;
// }

bool VehicleCt::is_event(const sct_core::ct_state &x, double t) {
    bool events =false;

    if (is_threshold_crossed(x) != 0) {
        events=true;
    }

    return events;
}

int VehicleCt::is_threshold_crossed(const sct_core::ct_state &x){
    double speed = x[1];
    double throttle_percentage = get_throttle(0.0)*100;
    double up_th = th_calc.getUpperThreshold(gear_in.read(), throttle_percentage);
    double down_th = th_calc.getLowerThreshold(gear_in.read(), throttle_percentage);

    int threshold_crossed = check_thresholds(up_th, down_th, speed);

    return threshold_crossed;
}




// void VehicleCt::generate_outputs(bool state_event_located, 
//             std::unordered_map<int, bool> events) {
//     double speed = x[1];


//     // std::cout << "Generating output " << sc_core::sc_time_stamp().to_seconds() << "" << std::endl;
//     if (state_event_located) {
//         int threshold_crossed = (*events.begin()).first;
//         last_written_threshold_out = threshold_crossed;
//         threshold_out.write( (ShiftingThresholds) last_written_threshold_out);
//         update_out->notify(sc_core::SC_ZERO_TIME);
//     }
//     else{        
//         // std::cout << "Th_out " << threshold_out.read() << std::endl;        
//     }

//     speed_out.write(speed);
// }

void VehicleCt::generate_outputs(bool state_event_located, 
            bool events) {
    
    double speed = x[1];


    // std::cout << "Generating output " << sc_core::sc_time_stamp().to_seconds() << "" << std::endl;
    if (state_event_located && events) {
        int threshold_crossed = is_threshold_crossed(x);
        last_written_threshold_out = threshold_crossed;
        threshold_out.write( (ShiftingThresholds) last_written_threshold_out);
        update_out->notify(sc_core::SC_ZERO_TIME);
    }
    else{        
        // std::cout << "Th_out " << threshold_out.read() << std::endl;        
    }

    speed_out.write(speed);
}

void VehicleCt::write_internal_data(double up_th, double down_th){
    (*myfile) << sc_core::sc_time_stamp().to_seconds() << "\t" << up_th << "\t" << down_th << std::endl;
}

double VehicleCt::get_throttle(double t) {
    return 0.30;
}

VehicleCt::~VehicleCt(){  
    #ifdef WRITE_INTERNAL_DATA_TO_FILE    
        myfile->close();
        delete myfile;
    #endif
}

int VehicleCt::check_thresholds(double up_th, double down_th, double speed){
    switch (last_written_threshold_out) {
        case speed_less_th_down : 
            if (speed > down_th) { // speed > down_th
                return speed_greater_th_down;
            }
            break; 
        case speed_greater_th_up : 
            if (speed < up_th) { //speed < up_th
                return speed_less_th_up;
            }
            break;
        case speed_greater_th_down:
        case speed_less_th_up:
            if (speed < down_th){ //speed < down_th
                return speed_less_th_down;
            }
            if (speed > up_th) { // speed > up_th
                return speed_greater_th_up;
            }
            break;
        default:
            break;
    }
    return 0;
}