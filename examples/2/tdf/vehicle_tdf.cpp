#include "vehicle_tdf.h"

VehicleTdf::VehicleTdf(sc_core::sc_module_name name)
    :   gear_ratio_in("gear_ratio_in"),
        gear_in("gear_in"), speed_out("speed_out"), 
        threshold_out("threshold_out"), equations(), timestep(TIMESTEP, sc_core::SC_MS)
{

    solver = new sct_kernel::numerical_solver();
    threshold_out.initialize_de_signal(speed_less_th_down);
}

void  VehicleTdf::set_attributes() {
    set_timestep(timestep);
}

void  VehicleTdf::processing() {

    double t_start = sc_core::sc_time_stamp().to_seconds();
    double t_end = t_start + get_timestep().to_seconds();

    equations.set_gear_ratio(gear_ratio_in.read());

    solver->integrate(
            equations.get_derivatives_func(), 
            equations.get_state(),
            t_start,
            t_end,
            equations.get_state_event_observer(), 
            [](double t, const sct_core::ct_state &x) { return; }
        );

    
    double speed = (equations.get_backup_state())[1];
    generate_outputs(speed);
    
    equations.backup_state();
}


void VehicleTdf::generate_outputs(double speed) {    
    int threshold_crossed = equations.check_thresholds(gear_in.read(), speed,       threshold_out->read());
    if (threshold_crossed) {
        std::cout << "Threshold crossed at " << sc_core::sc_time_stamp().to_seconds() << "speed " << speed << "ne " << (equations.get_backup_state())[0] << std::endl;
        threshold_out.write( (ShiftingThresholds) threshold_crossed);
        update_out.write(!update_out->read());
    }

    speed_out.write(speed); 
}
