#include "bool_to_event_converter.h"

BoolToEventConverter::BoolToEventConverter(sc_core::sc_module_name name){
    SC_METHOD(convert);
    sensitive << bool_in;
}

void BoolToEventConverter::convert(){
    event_out->notify(sc_core::SC_ZERO_TIME);
}