#ifndef SYNAPSE_H
#define SYNAPSE_H

#include <systemc>
#include "signal_rst.h"
#include <mutex>

class synapse_if: public virtual sc_core::sc_interface {
    public:
        virtual void increment() = 0;
};

class synapse_out : public sc_core::sc_port<synapse_if, 0> {

    public:
        synapse_out(sc_core::sc_module_name name) : sc_core::sc_port<synapse_if, 0>(name) {

        }
};

class synapse : public signal_rst, public synapse_if {
    public:
        synapse() {
            count = 0;
            should_update = false;
        }
        
        void increment() {
            // const std::lock_guard<std::mutex> lock(update_mutex);
            count++;


            if (!should_update) {
                async_request_update();
                should_update = true;
            }
            
        }


    protected:

        void update() {
            write(count);
            signal_rst::update();
            count = 0;
            should_update = false;
        }
        

    private: 
        int count;
        bool should_update;
        // std::mutex update_mutex;
};


class synapse_in : public signal_rst_in {
};

#endif 