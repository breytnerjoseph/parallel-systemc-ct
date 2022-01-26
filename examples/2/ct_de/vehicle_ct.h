#ifndef VEHICLE_CT_H
#define VEHICLE_CT_H

#include <systemc>
#include <cmath>
#include <fstream>

#include "engine_torque_calculator.h"
#include "torque_constants_calculator.h"
#include "thresholds_calculator.h"
#include "shifting_thresholds.h"

#include "ct_module.h"

// #define WRITE_INTERNAL_DATA_TO_FILE 1

// Order of the system of diff. equations
#define ORDER 3 
// Max. timestep to be taken for synchronization 
#define DELTA_T 0.2 // seconds

class VehicleCt : public sct_core::ct_module { 
    public:
        // gear ratio from transmission control unit
        sc_core::sc_in<double> gear_ratio_in; 
        sc_core::sc_in<double> gear_in;
        
        sc_core::sc_out<double> speed_out;
        sc_core::sc_port<sc_core::sc_event_queue_if> update_out;
        sc_core::sc_out<ShiftingThresholds> threshold_out;

        VehicleCt(sc_core::sc_module_name name,
            bool use_adaptive = true, bool avoid_rollback = true, 
            double sync_step = DELTA_T, bool parallel_execution = false 
        );

        ~VehicleCt();

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
        double ub = 0.0; // Braking;

        void set_initial_conditions();
        void set_sync_parameters();

        void get_derivatives(bool use_input_checkpoints,
            const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t );

        // Threshold crossing conditions.
        // std::unordered_map<int, bool> is_event(const sct_core::ct_state &x, double t = 0);

        bool is_event(const sct_core::ct_state &x, double t = 0);

        // void generate_outputs(bool state_event_located, 
        //     std::unordered_map<int, bool> events
        // );

        void generate_outputs(bool state_event_located, 
            bool events
        );

        // OPTIONAL METHODS
        void set_attributes();

    private:
        EngineTorqueCalculator te_calculator;
        TorqueConstantsCalculator constants_calculator;
        ThresholdsCalculator th_calc;

        int check_thresholds(double up_th, double down_th, double speed);
        double get_throttle(double t) ;

        int is_threshold_crossed(const sct_core::ct_state &x);

        // Little DE state
        int last_written_threshold_out;

        // To collect internal information: up and down thresholds
        std::ofstream *myfile;
        void write_internal_data(double up_th, double down_th);

        enum event_ids {GTD = speed_greater_th_down, LTU = speed_less_th_up, LTD = speed_less_th_down, GTU = speed_greater_th_up };

        // Sync. parameters
        bool use_adaptive, avoid_rollback; 
        double sync_step;
};

#endif
