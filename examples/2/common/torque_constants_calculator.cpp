#include "torque_constants_calculator.h"

TorqueConstantsCalculator::TorqueConstantsCalculator() 
    : interp_ktc_tbl(get_ni_ne_breakpoints()), interp_ctr_tbl(get_ni_ne_breakpoints())
{
    int i = 0;

    std::vector<double> ni_ne_breakpoints(get_ni_ne_breakpoints());

    for (auto x : ni_ne_breakpoints) {
        interp_ktc_tbl.loadDatum(x, ktc_data[i]);
        interp_ctr_tbl.loadDatum(x, ctr_data[i]);
        i++; 
    }

}

double TorqueConstantsCalculator::get_capacity_factor(double vehicle_speed, double engine_rpm, double gear_ratio) const {

    double ni_ne = get_ni_ne(vehicle_speed, engine_rpm, gear_ratio);

    return interp_ktc_tbl.getValue(ni_ne);
}

double TorqueConstantsCalculator::get_torque_ratio(double vehicle_speed, double engine_rpm, double gear_ratio) const {
    double ni_ne = get_ni_ne(vehicle_speed, engine_rpm, gear_ratio);
    return interp_ctr_tbl.getValue(ni_ne);
} 

double TorqueConstantsCalculator::get_capacity_factor(double speed_ratio) const {
    return interp_ktc_tbl.getValue(speed_ratio);
}

double TorqueConstantsCalculator::get_torque_ratio(double speed_ratio) const {
    return interp_ctr_tbl.getValue(speed_ratio);
} 

double TorqueConstantsCalculator::get_ni_ne(double vehicle_speed, double engine_rpm, double gear_ratio) const {
    if (engine_rpm == 0) {
        std::cout << "TorqueConstantsCalculator::get_ni_ne engine_rpm must be different from 0" << std::endl;
        abort();
    }
    
    return vehicle_speed*gear_ratio*(1.0/(2.0*3.14159265359))/engine_rpm;
}

std::vector<double> TorqueConstantsCalculator::get_ni_ne_breakpoints() const {
    return std::vector<double>({
        0.0000, 0.1000, 0.2000, 0.3000, 0.4000, 0.5000, 0.6000,
        0.7000, 0.8000, 0.8100, 0.8200, 0.8300, 0.8400, 0.8500,
        0.8600, 0.8700, 0.8800, 0.8900, 0.9000, 0.9200, 0.9400});
}

