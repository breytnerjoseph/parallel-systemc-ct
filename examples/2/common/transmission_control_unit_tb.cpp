
#include "transmission_control_unit_tb.h"

TcuTb::TcuTb(sc_core::sc_module_name name) 
            : th_calc() 
{
    import_data("gradual_acc_data/throttle_data.txt", throttle_data);
    import_data("gradual_acc_data/gear_data.txt", gear_data);
    import_data("gradual_acc_data/speed_data.txt", speed_data);
    import_data("gradual_acc_data/up_th_data.txt", up_th_data);
    import_data("gradual_acc_data/down_th_data.txt", down_th_data);

    SC_THREAD(generate_tests);
    threshold_out.initialize(speed_less_th_down);
}
    
void TcuTb::generate_tests(){
    double up_th;
    double down_th;
    int threshold_crossed; 
    bool success = true;

    unsigned i = 0;
    while(i <  throttle_data.size() ){
        up_th = th_calc.getUpperThreshold(gear_in.read(), throttle_data[i]);
        down_th = th_calc.getLowerThreshold(gear_in.read(), throttle_data[i]);
        double allowed_error = 0.01;
        
        if (abs(up_th - up_th_data[i]) > allowed_error) {
            std::cout << "UP thresholds are different " << up_th << " != " << up_th_data[i] << " @ " << sc_core::sc_time_stamp() << std::endl; 
        }
        if (abs(down_th - down_th_data[i]) > allowed_error) {
            std::cout << "DOWN thresholds are different " << down_th << " != " << down_th_data[i] << " @ " << sc_core::sc_time_stamp() << std::endl; 
        }

        threshold_crossed = check_thresholds(up_th, down_th, speed_data[i]);
        if (threshold_crossed) {
            threshold_out.write( (ShiftingThresholds) threshold_crossed);
            update_out->notify(sc_core::SC_ZERO_TIME);
        }

        if(gear_in.read() != gear_data[i]){
            std::cout << "Gear in: " << gear_in.read() << " Gear data " << gear_data[i] << "   " << sc_core::sc_time_stamp() << std::endl;
            success = false;
        }

        // Timestep MATLAB data
        wait(40, sc_core::SC_MS); 
        i++;
    }

    if (success) {
        std::cout << "Enhorabuena! Gear changes are identical to those obtained by MATLAB" << std::endl;
    }
}



void TcuTb::import_data(std::string filename, std::vector<double> &data){
    std::ifstream file;
    file.open(filename);
    std::string line;

    if (file.is_open()) {
        while (std::getline(file,line)) {
            double val = std::stod(line);
            data.push_back(val);
        }
        file.close();
    }
}

int TcuTb::check_thresholds(double up_th, double down_th, double speed){
    switch (threshold_out.read()) {
        case speed_less_th_down : 
            if (speed > down_th) {
                return speed_greater_th_down;
            }
            break; 
        case speed_greater_th_up : 
            if (speed < up_th) {
                return speed_less_th_up;
            }
            break;
        case speed_greater_th_down:
        case speed_less_th_up:
            if (speed < down_th){
                return speed_less_th_down;
            }
            if (speed > up_th){
                return speed_greater_th_up;
            }
            break;
        default:
            break;
    }
    return 0;
}