#ifndef BOOL_TO_EVENT_CONVERTER_H
#define BOOL_TO_EVENT_CONVERTER_H

#include <systemc>

// Generate events in the event_out (event queue) at every value change in the 
// boolean signal

class BoolToEventConverter : public sc_core::sc_module {
    SC_HAS_PROCESS(BoolToEventConverter);

    public: 
        sc_core::sc_in<bool> bool_in;
        sc_core::sc_port<sc_core::sc_event_queue_if> event_out;

        BoolToEventConverter(sc_core::sc_module_name name);

    private:
        void convert();
};

#endif