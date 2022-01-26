#ifndef GEAR_STATE_TB_H
#define GEAR_STATE_TB_H

#include <systemc>

class GearStateTb : public sc_core::sc_module {
    SC_HAS_PROCESS(GearStateTb);
    
    public: 
        // Ports
        sc_core::sc_port<sc_core::sc_event_queue_if> update_out;
        sc_core::sc_out<bool> direction_out;
        sc_core::sc_in<double> gear_in;
        sc_core::sc_in<double> gear_ratio_in;

        GearStateTb(sc_core::sc_module_name name) : 
            update_out("update_out"), direction_out("direction_out"), 
            gear_in("gear_in"), gear_ratio_in("gear_ratio_in")
        {
            
            SC_THREAD(generate_test);

            direction_out.initialize(false);
        }
    
    private:
        void generate_test(){
            // Pass from 1 to 2
            update_out->notify(sc_core::SC_ZERO_TIME);
            direction_out.write(true);
            wait(gear_in.default_event());
            assert(gear_in.read() == 2);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);

            // Pass from 2 to 3
            update_out->notify(sc_core::SC_ZERO_TIME);
            wait(gear_in.default_event());
            assert(gear_in.read() == 3);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);

            // Pass from 3 to 2
            update_out->notify(sc_core::SC_ZERO_TIME);
            direction_out.write(false);
            wait(gear_in.default_event());
            assert(gear_in.read() == 2);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);

            // Pass from 2 to 3
            update_out->notify(sc_core::SC_ZERO_TIME);
            direction_out.write(true);
            wait(gear_in.default_event());
            assert(gear_in.read() == 3);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);

            // Pass from 3 to 4
            update_out->notify(sc_core::SC_ZERO_TIME);
            wait(gear_in.default_event());
            assert(gear_in.read() == 4);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);

            // Pass from 4 to 4
            update_out->notify(sc_core::SC_ZERO_TIME);
            wait(5, sc_core::SC_SEC);
            assert(gear_in.read() == 4);
            assert(gear_ratio_in.read() == gear_ratios[gear_in.read() - 1]);


            std::cout << "Enhorabuena! All GearState tests have passed" << std::endl;
        }

        double gear_ratios[4] = {2.34, 1.450, 1.000, 0.68};

};

#endif