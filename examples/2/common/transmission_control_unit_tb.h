#ifndef TRANSMISSION_CONTROL_UNIT_TB_H
#define TRANSMISSION_CONTROL_UNIT_TB_H

#include <systemc>
#include "shifting_thresholds.h"
#include "thresholds_calculator.h"
#include <iostream>

class TcuTb : public sc_core::sc_module {
    SC_HAS_PROCESS(TcuTb);
    
    public: 
        // Ports
        sc_core::sc_port<sc_core::sc_event_queue_if> update_out;
        sc_core::sc_out<ShiftingThresholds> threshold_out;
        sc_core::sc_in<double> gear_in;

        TcuTb(sc_core::sc_module_name name);
    
    private:
        void generate_tests();

        ThresholdsCalculator th_calc;

        // To contain MATLAB generated data
        std::vector<double> throttle_data, gear_data, speed_data, up_th_data, down_th_data; 
        
        void import_data(std::string filename, std::vector<double> &data);

        int check_thresholds(double up_th, double down_th, double speed);
};

#endif