#ifndef VEHICLE_TDF_H
#define VEHICLE_TDF_H

#define TIMESTEP 100 // Milliseconds

#include <systemc>
#include <systemc-ams>
#include "numerical_solver.h"
#include "shifting_thresholds.h"
#include "vehicle_equations.h"


class VehicleTdf : public sca_tdf::sca_module { 
    public:
        sca_tdf::sca_de::sca_in<double> gear_ratio_in;
        sca_tdf::sca_de::sca_in<double> gear_in;
        sca_tdf::sca_de::sca_out<double> speed_out;
        sca_tdf::sca_de::sca_out<bool> update_out;
        sca_tdf::sca_de::sca_out<ShiftingThresholds> threshold_out;

        VehicleTdf(sc_core::sc_module_name name);

        void generate_outputs(double speed);
        void processing();
        void set_attributes();

    private: 
        sct_kernel::numerical_solver *solver;
        VehicleEquations equations;
        sca_core::sca_time timestep;
};

#endif