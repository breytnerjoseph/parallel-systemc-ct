
#include "engine_torque_calculator.h"

EngineTorqueCalculator::EngineTorqueCalculator()
    : interp_te_tbl(get_ne_breakpoints(), get_throttle_breakpoints()),
        throttle(get_throttle_breakpoints()),
        ne(get_ne_breakpoints()) 
{
    std::vector<std::vector<double> > te_table(10); 

    for( unsigned i = 0; i < throttle.size(); i++){
        te_table[i].resize(ne.size());
        for( unsigned j = 0; j < ne.size(); j++){
            te_table[i][j] = _te_table[i][j];
        }
    }

    interp_te_tbl.loadData(te_table);
}

double EngineTorqueCalculator::get_torque(double ne, double throttle) const { 
    return interp_te_tbl.getValue(ne, throttle);
}

std::vector<double> EngineTorqueCalculator::get_ne_breakpoints() const {
    std::vector<double> result;
    for (int i = 800; i <= 4800; i+=400 ) {
        result.push_back( (double) i);
    }
    return result;
}

std::vector<double> EngineTorqueCalculator::get_throttle_breakpoints() const {
    return std::vector<double>({0.0, 0.20, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1});
}