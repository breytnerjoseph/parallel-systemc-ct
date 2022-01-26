#include "vehicle_equations.h"

VehicleEquations::VehicleEquations() 
    : te_calculator(), th_calc() {
    set_initial_conditions();

    // Lambda function for differential equations
    diff_equations_func = [&](const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t){ 
        get_derivatives(x, dxdt, t);
    };

    // Lambda function for threshold detection conditions
    state_event_observer =  [&](const sct_core::ct_state &x, const double t){ 
        return false;
    };
}


void VehicleEquations::set_initial_conditions() {
    x[0] = 1000;
    x[1] = x[2] = 0;

    x_backup = x;
}


void VehicleEquations::get_derivatives( 
    const sct_core::ct_state &x, 
    sct_core::ct_state &dxdt,
    double t)
{
    double Ne = x[0];
    double v = x[1];
    double Pb = x[2];

    double throttle = get_throttle(0.0);
    double ub = 0.0; // Braking
    
    double Rtr = gear_ratio;

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


bool VehicleEquations::set_gear_ratio(double gear_ratio_in) {
    gear_ratio = gear_ratio_in;
    return true;
}


sct_core::ct_state &VehicleEquations::get_state() {
    return this->x;
}


bool VehicleEquations::set_state(sct_core::ct_state x) {
    this->x = x;
    return true;
}


void VehicleEquations::backup_state(){
    x_backup = x;
}


int VehicleEquations::check_thresholds(double gear_in, double speed, 
    int current_threshold) 
{
    double throttle_percentage = get_throttle(0.0)*100;
    double up_th = th_calc.getUpperThreshold(gear_in, throttle_percentage);
    double down_th = th_calc.getLowerThreshold(gear_in, throttle_percentage);


    switch (current_threshold) {
        case speed_less_th_down : 
            if (speed > down_th) {
                return speed_greater_th_down;
            }
            break; 
        case speed_greater_th_up : 
            if (speed < up_th) {
                return speed_less_th_up;
            }
            break;
        case speed_greater_th_down:
        case speed_less_th_up:
            if (speed < down_th){
                return speed_less_th_down;
            }
            if (speed > up_th){
                return speed_greater_th_up;
            }
            break;
        default:
            break;
    }
    return 0;
}


double VehicleEquations::get_throttle(double t) const {
    return 0.30;
}


std::function<void( const sct_core::ct_state &x,
    sct_core::ct_state &dxdt, 
    double t)> &VehicleEquations::get_derivatives_func() {
        return diff_equations_func;
}


std::function<bool(const sct_core::ct_state &, 
    const double)> &VehicleEquations::get_state_event_observer() {
        return state_event_observer;
} 


const sct_core::ct_state &VehicleEquations::get_backup_state(){
    return this->x_backup;
}