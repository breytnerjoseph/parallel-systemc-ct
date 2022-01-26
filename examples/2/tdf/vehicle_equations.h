#ifndef VEHICLE_EQUATIONS_H
#define VEHICLE_EQUATIONS_H

#include <cmath>
#include <vector>
#include <functional>
#include "ct_state.h"
#include "engine_torque_calculator.h"
#include "torque_constants_calculator.h"
#include "thresholds_calculator.h"
#include "shifting_thresholds.h"

class VehicleEquations { 
    public:
        // System parameters
        double m = 1500.5;  // kg, vehicle mass
        double r = 0.326;   // m, wheel radius
        double Rf = 3.28;   // Final driver ratio
        double Iei = 0.224; // Kg*m2, moment of intertia of engine and torque converter 
        double taub = 0.2;  // Lumped lag-servo valve and the hydraulic system
        double rhoACd = 0.98; // Aerodynamic force coefficient
        double Cr = 0.015; // Rolling resistance coefficient
        double Kc = 1;  //  Pressure gain
        double Kb = 20; // Nm/bar, Lumped gain for entire brake system
        double g = 9.8; // m/s2, gravitational acceleration
        double vw = 0.0; // Assume wind v is negligible
        double theta = 0.0; // Assume no slope in road

        // Constructor and initialization method
        VehicleEquations();
        void set_initial_conditions();

        // Higher order functions (almost): they 
        // return pointers to functions to be used 
        // by the numerical solver.
        std::function<void( const sct_core::ct_state &x,
            sct_core::ct_state &dxdt, 
            double t)> &get_derivatives_func();
        std::function<bool(const sct_core::ct_state &, 
            const double)> &get_state_event_observer();

        // State-related method
        sct_core::ct_state &get_state();
        bool set_state(sct_core::ct_state x);        
        void backup_state();
        const sct_core::ct_state &get_backup_state();

        // Variable parameter determined by
        // DE Transmission Control Unit
        bool set_gear_ratio(double);
        
        // Method containing the threshold detection
        // condition
        int check_thresholds(double gear_in, double speed, 
            int current_threshold);

    private: 
        // Calculators for the variable parameters
        // of the non-linear system
        EngineTorqueCalculator te_calculator;
        TorqueConstantsCalculator constants_calculator;
        ThresholdsCalculator th_calc;

        // State and state backup variables
        sct_core::ct_state x, x_backup;
        
        // Gear ratio determined by the 
        // DE Transmission Control Unit  
        double gear_ratio;

        // Diff. equations and state event observer 
        // function pointers
        std::function<void(const sct_core::ct_state &x, sct_core::ct_state &dxdt, 
            double t)> diff_equations_func;
        std::function<bool(const sct_core::ct_state &, const double)> state_event_observer;

        // Method that models the throttle input function
        double get_throttle(double t) const;

        // Method that models the ODE system
        void get_derivatives( const sct_core::ct_state &x, 
            sct_core::ct_state &dxdt,
            double t);
};

#endif