#ifndef TORQUE_CONSTANTS_CALCULATOR_H
#define TORQUE_CONSTANTS_CALCULATOR_H

#include <tuple>
#include <vector>
#include <iostream>

#include "look_up_table_1d.h"


class TorqueConstantsCalculator{
    public:    
        TorqueConstantsCalculator();
        
        double get_capacity_factor(double vehicle_speed, double engine_rpm, double gear_ratio) const; 
        double get_torque_ratio(double vehicle_speed, double engine_rpm, double gear_ratio) const; 

        double get_capacity_factor(double speed_ratio) const; 
        double get_torque_ratio(double speed_ratio) const; 
        
    private:
        LookUpTable1D interp_ktc_tbl;
        LookUpTable1D interp_ctr_tbl;

        double ktc_data[21] = {
            137.4652, 137.0650, 135.8644, 135.6644, 137.5653, 140.3666, 145.2689,
            152.8725, 162.9773, 164.2779, 166.1788, 167.9797, 170.0807, 172.7820,
            175.3832, 179.5852, 183.5871, 189.8901, 197.6938, 215.9024, 244.5160 };

        double ctr_data[21] = {
            2.2320, 2.0750, 1.9750, 1.8460, 1.7200, 1.5640, 1.4090,
            1.2540, 1.0960, 1.0800, 1.0610, 1.0430, 1.0280, 1.0120,
            1.002,  1.002,  1.001,  0.9980, 0.9990, 1.001,  1.002 };

        std::vector<double> get_ni_ne_breakpoints() const;

        double get_ni_ne(double vehicle_speed, double engine_rpm, double gear_ratio) const;
};

#endif