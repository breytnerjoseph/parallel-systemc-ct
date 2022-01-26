#ifndef SIGNAL_RST_H
#define SIGNAL_RST_H

#include <systemc>

class signal_rst_if: public virtual sc_core::sc_interface {
};

// Double signals that emit event at any writing, independently of the value.
class signal_rst :  public virtual sc_core::sc_signal<double, sc_core::SC_MANY_WRITERS>,public signal_rst_if {
    public:
        signal_rst() {
             last_update_time = 0;
        }
        
        bool event() {
            return (last_update_time == sc_core::sc_delta_count());
        }

        const sc_core::sc_event &get_update_event() const {
            return update_event;
        }

    protected:

        void update() {
            sc_core::sc_signal<double, sc_core::SC_MANY_WRITERS>::update();
            last_update_time = sc_core::sc_delta_count();
            update_event.notify(sc_core::SC_ZERO_TIME); // Next delta cycle
        }
        
    private:
        sc_dt::uint64 last_update_time;
        sc_core::sc_event update_event;

};


class signal_rst_in : public sc_core::sc_port<signal_rst> {

    public: 
        bool event() {
            return this->operator->()->event();
        }

        const sc_core::sc_event &default_event() {
            return this->operator->()->get_update_event();
        }

        const double &read() {
            return this->operator->()->read();
        }

};

class signal_rst_out : public sc_core::sc_port<signal_rst_if> {
};


#endif 