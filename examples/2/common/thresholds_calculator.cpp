#include "thresholds_calculator.h"
#include<iostream>

ThresholdsCalculator::ThresholdsCalculator() 
    : interp_up_tbl(get_gear_breakpoints(), get_up_th_breakpoints()), 
        interp_down_tbl(get_gear_breakpoints(), get_down_th_breakpoints()),
        up_th_bp(get_up_th_breakpoints()),
        down_th_bp(get_down_th_breakpoints()),
        gears(get_gear_breakpoints())
{

    // Load data arrays into vector of vectors
    std::vector<std::vector<double> > up_table(6), down_table(6); 
    for( unsigned i = 0; i < up_th_bp.size(); i++){
        up_table[i].resize(4);
        down_table[i].resize(4);
        for( unsigned j = 0; j < gears.size(); j++){
            up_table[i][j] = _up_table[i][j];
            down_table[i][j] = _down_table[i][j];
        }
    }

    interp_up_tbl.loadData(up_table);
    interp_down_tbl.loadData(down_table);

}

double ThresholdsCalculator::getUpperThreshold(int gear, double throttle) const {
    return  interp_up_tbl.getValue( (double) gear, throttle);
} 

double ThresholdsCalculator::getLowerThreshold(int gear, double throttle) const {
    return interp_down_tbl.getValue( (double) gear, throttle);
}

std::vector<double> ThresholdsCalculator::get_up_th_breakpoints() const {
    return std::vector<double>({0, 11.176, 15.6464, 22.352, 40.2336, 44.704});
}

std::vector<double> ThresholdsCalculator::get_down_th_breakpoints() const {
    return std::vector<double>({0, 2.2352, 17.8816, 22.352, 40.2336, 44.704});
}

std::vector<double> ThresholdsCalculator::get_gear_breakpoints() const {
    return std::vector<double>({1, 2, 3, 4});
}
