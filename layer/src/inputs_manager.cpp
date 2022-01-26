#include "../include/inputs_manager.h"

inputs_manager::inputs_manager(){
    mode = current_value;
    already_calculated_sensitivity_list = false;
    already_calculated_event_list = false; 
}

void inputs_manager::use_checkpoints(bool use_it) {
    if(use_it) {
        this->set_mode(checkpoint);
    }
    else{
        this->set_mode(current_value);
    }
}

bool inputs_manager::operator[](const sc_core::sc_in<bool> &port) {
    if (mode==current_value) {
        return this->get_current_value_bool((long int) &port);
    }
    
    if(mode==checkpoint) {
        return this->get_checkpoint_value_bool((long int) &port);
    }

    throw std::invalid_argument("port");
}

double inputs_manager::operator[](const sc_core::sc_in<double> &port) {
    if (mode==current_value) {
        return this->get_current_value_double((long int) &port);
    }
    
    if(mode==checkpoint) {
        return this->get_checkpoint_value_double((long int) &port);
    }

    throw std::invalid_argument("port");
}

double inputs_manager::operator[](const signal_rst_in &port) {
    if (mode==current_value) {
        return this->get_current_value_signal_rst((long int) &port);
    }
    
    if(mode==checkpoint) {
        return this->get_checkpoint_value_signal_rst((long int) &port);
    }

    throw std::invalid_argument("port");
}


bool inputs_manager::operator[](const sc_core::sc_port<sc_core::sc_signal_in_if<bool> > &port){
    const sc_core::sc_in<bool> *port_ptr = static_cast<const sc_core::sc_in<bool> *>(&port);
    return this->operator[](*port_ptr);
}

double inputs_manager::operator[](const sc_core::sc_port<sc_core::sc_signal_in_if<double> > &port){
    const sc_core::sc_in<double> *port_ptr = static_cast<const sc_core::sc_in<double> *>(&port);
    return this->operator[](*port_ptr);
}



void inputs_manager::set_mode(modes_of_operation new_mode) {
    mode = new_mode;
}

void inputs_manager::register_input_ports(const std::vector<sc_core::sc_object* > &child_objects) {
    // Iterate over child objects to store input ports
    for (sc_core::sc_object * object: child_objects) { 
        if (is_input_port(object) && !store_port(object)) {
            std::cout << "*** Failed to store input port in synchronization layer";
            sc_core::sc_stop();
        }
    }
}

void inputs_manager::create_checkpoint() {
    // std::cout << "inputs_manager::create_checkpoint called" << std::endl;
    for (auto &el : boolean_inputs) {
        // el is of type 
        // std::pair<std::string, std::pair< sc_core::sc_in<bool>*, bool>>
        // The first .second returns an object of type 
        // std::pair< sc_core::sc_in<bool>*, bool>
        // The second one returns an object of type bool  
        el.second.second = el.second.first->read();
    }

    for (auto &el : double_inputs) {
        // See comment for boolean case above
        el.second.second = el.second.first->read();
    }

    for (auto &el : signal_rst_inputs) {
        // See comment for boolean case above
        el.second.second = el.second.first->read();
    }
}

bool inputs_manager::have_inputs_changed() const {
    return have_boolean_inputs_changed() || have_double_inputs_changed() || have_signal_rst_inputs_changed();
}

bool inputs_manager::have_boolean_inputs_changed() const {
    for (auto el : boolean_inputs) {
        if (el.second.second != el.second.first->read()) {              
            return true;
        }
    }
    return false;
}

bool inputs_manager::have_double_inputs_changed() const {
    for (auto el : double_inputs) {
        // if (el.second.second != el.second.first->read()) {
        //     std::cout << "double inputs changed " << std::endl;
        //     return true;
        // }

        if (el.second.first->event()) {
            return true;
        }
    }
    return false;
}

bool inputs_manager::have_signal_rst_inputs_changed() const {
    for (auto el : signal_rst_inputs) {
        if (el.second.first->event()) {
            return true;
        }
    }
    return false;
}


const sc_core::sc_event_or_list & inputs_manager::get_sensitivity_list() {
    
    if (!already_calculated_sensitivity_list) {
        for (auto event_ptr : get_event_list()) {
            sensitivity_list |= *event_ptr;
        }
        already_calculated_sensitivity_list = true;
    }
    
    return sensitivity_list;
}


const std::vector<const sc_core::sc_event *> &inputs_manager::get_event_list() {
    if (!already_calculated_event_list) {
        for (auto el : boolean_inputs) {
            event_list.push_back(&el.second.first->default_event());
        }
        for (auto el : double_inputs) {
            event_list.push_back(&el.second.first->default_event());
        }
        for (auto el : signal_rst_inputs) {
            event_list.push_back(&el.second.first->default_event());
        }
        already_calculated_event_list = true;
    }
    return event_list;
}



//////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS 
//////////////////////////////////////////////////////////////////

bool inputs_manager::is_input_port(sc_core::sc_object *object) {
    return dynamic_cast<sc_core::sc_in<bool> *>(object) != NULL || 
    dynamic_cast<sc_core::sc_in<double> *>(object) != NULL || 
    dynamic_cast<signal_rst_in *>(object) != NULL;
 //   return std::string(object->kind()) == "sc_in" || std::string(object->kind);
}

bool inputs_manager::store_port(sc_core::sc_object * port) {
    return store_boolean_port(port) || store_double_port(port) || store_signal_rst_port(port);
}


bool inputs_manager::store_boolean_port(sc_core::sc_object * port) {
    sc_core::sc_in<bool> *port_ptr;
    if((port_ptr = dynamic_cast<sc_core::sc_in<bool>* >(port))) {
        auto port_pointer_and_backup = std::make_pair(
                port_ptr,
                port_ptr->read() 
        );

        auto element = std::make_pair(
            // Key
            (long int) port_ptr, 
            // Value: pointer to the port and checkpoint value
            port_pointer_and_backup 
        );
        
        boolean_inputs.insert(element);
        return true;
    }
    return false;
}

bool inputs_manager::store_double_port(sc_core::sc_object * port) {
    sc_core::sc_in<double> *port_ptr;
    if((port_ptr = dynamic_cast<sc_core::sc_in<double>* >(port))) {
        auto port_pointer_and_backup = std::make_pair(
                port_ptr,
                port_ptr->read() 
        );

        auto element = std::make_pair(
            // Key
            (long int) port_ptr, 
            // Value: pointer to the port and checkpoint value
            port_pointer_and_backup 
        );
        
        double_inputs.insert(element);
        return true;
    }
    return false;
}

bool inputs_manager::store_signal_rst_port(sc_core::sc_object * port) {
    signal_rst_in *port_ptr;
    if((port_ptr = dynamic_cast<signal_rst_in* >(port))) {
        auto port_pointer_and_backup = std::make_pair(
                port_ptr,
                port_ptr->read() 
        );

        auto element = std::make_pair(
            // Key
            (long int) port_ptr, 
            // Value: pointer to the port and checkpoint value
            port_pointer_and_backup 
        );
        
        signal_rst_inputs.insert(element);
        return true;
    }
    return false; 
}

// bool inputs_manager::get_current_value_bool(const std::string &port_name) {
//     return boolean_inputs[port_name].first->read();
// }


// double inputs_manager::get_current_value_double(const std::string &port_name) {
//     return double_inputs[port_name].first->read();

// }

// double inputs_manager::get_current_value_signal_rst(const std::string &port_name) {
//     return signal_rst_inputs[port_name].first->read();

// }

// bool inputs_manager::get_checkpoint_value_bool(const std::string &port_name) {
//     return boolean_inputs[port_name].second;
// }

// double inputs_manager::get_checkpoint_value_double(const std::string &port_name) {
//     return double_inputs[port_name].second;
// } 

// double inputs_manager::get_checkpoint_value_signal_rst(const std::string &port_name) {
//     return signal_rst_inputs[port_name].second;
// } 


bool inputs_manager::get_current_value_bool(long int port_name) {
    return boolean_inputs.at(port_name).first->read();
    // return boolean_inputs[port_name].first->read();
}


double inputs_manager::get_current_value_double(long int port_name) {
    return double_inputs.at(port_name).first->read();

}

double inputs_manager::get_current_value_signal_rst(long int port_name) {
    return signal_rst_inputs.at(port_name).first->read();

}

bool inputs_manager::get_checkpoint_value_bool(long int port_name) {
    return boolean_inputs.at(port_name).second;
}

double inputs_manager::get_checkpoint_value_double(long int port_name) {
    return double_inputs.at(port_name).second;
} 

double inputs_manager::get_checkpoint_value_signal_rst(long int port_name) {
    return signal_rst_inputs.at(port_name).second;
} 

