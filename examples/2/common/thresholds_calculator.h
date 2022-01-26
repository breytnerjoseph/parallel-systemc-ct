#ifndef THRESHOLDS_CALCULATOR_H
#define THRESHOLDS_CALCULATOR_H

#include <tuple>
#include <vector>

#include "look_up_table_2d.h"


class ThresholdsCalculator{
    public:    
        ThresholdsCalculator();

        double getUpperThreshold(int gear, double throttle) const; 
        double getLowerThreshold(int gear, double throttle) const; 

    private:
        LookUpTable2D interp_up_tbl;
        LookUpTable2D interp_down_tbl;

        std::vector<double> up_th_bp;
        std::vector<double> down_th_bp;
        std::vector<double> gears;

        double _up_table[6][4] = {
            {4.4704,	13.4112,	22.352,	    447040},
            {4.4704,	13.4112,	22.352,	    447040},
            {6.7056,	13.4112,	22.352,	    447040},
            {10.28192,	18.32864,	26.822,	    447040},
            {17.8816,	31.2928,	44.704,	    447040},
            {17.8816,	31.2928,	44.704,	    447040},
        };

        double _down_table[6][4] = {
            {0.0,	    2.2352,	    8.9408,	    15.6464},
            {0.0,	    2.2352,	    8.9408,	    15.6464},
            {0.0,	    2.2352,	    11.176,	    17.8816},
            {0.0,	    2.2352,	    13.4112,	22.352},
            {0.0,	    13.4112,	22.352,	    35.7632},
            {0.0,	    13.4112,	22.352,	    35.7632},
        };

        std::vector<double> get_gear_breakpoints() const;
        std::vector<double> get_down_th_breakpoints() const;
        std::vector<double> get_up_th_breakpoints() const;
};

#endif