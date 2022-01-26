#ifndef INPUTS_MANAGER_H
#define INPUTS_MANAGER_H

#include <map>
#include <systemc>
#include "signal_rst.h"

// Provisional class. Generalization 
// to be done when investigating the internals 
// of CT MoCs


// Inputs manager modes of operation:
// defines whether it will return the current value 
// of the input or the checkpoint by default

class inputs_manager {

    public:
        typedef enum {current_value, checkpoint} modes_of_operation;
        inputs_manager();
        void use_checkpoints(bool use_it);
        bool operator[](const sc_core::sc_in<bool> &port);
        bool operator[](const sc_core::sc_port<sc_core::sc_signal_in_if<bool> > &port);
        double operator[](const sc_core::sc_port<sc_core::sc_signal_in_if<double> > &port);
        double operator[](const sc_core::sc_in<double> &port);
        double operator[](const signal_rst_in &port);
        void set_mode(modes_of_operation new_mode);
        void register_input_ports(const std::vector<sc_core::sc_object* > &child_objects);
        void create_checkpoint();
        bool have_inputs_changed() const;
        bool have_boolean_inputs_changed() const;
        bool have_double_inputs_changed() const;
        bool have_signal_rst_inputs_changed() const;
        const sc_core::sc_event_or_list &get_sensitivity_list();
        const std::vector<const sc_core::sc_event *> &get_event_list();

    private:
        // std::unordered_map< std::string, std::pair< sc_core::sc_in<bool>*, bool> > boolean_inputs;
        // std::unordered_map< std::string, std::pair< sc_core::sc_in<double>*, double> > double_inputs;
        // // Double signals that emit event at writing independently of the value
        // std::unordered_map< std::string, std::pair< signal_rst_in*, double> > signal_rst_inputs;

        std::map< int, std::pair< sc_core::sc_in<bool>*, bool> > boolean_inputs;
        std::map< int, std::pair< sc_core::sc_in<double>*, double> > double_inputs;
        // Double signals that emit event at writing independently of the value
        std::map< int, std::pair< signal_rst_in*, double> > signal_rst_inputs;

        std::vector<const sc_core::sc_event *> event_list;
        sc_core::sc_event_or_list sensitivity_list;
        modes_of_operation mode;

        bool already_calculated_sensitivity_list;
        bool already_calculated_event_list;

        //////////////////////////////////////////////////////////////////
        // UTILITY FUNCTIONS 
        //////////////////////////////////////////////////////////////////
        bool is_input_port(sc_core::sc_object *object);
        bool store_port(sc_core::sc_object * port);
        bool store_boolean_port(sc_core::sc_object * port);
        bool store_double_port(sc_core::sc_object * port);
        bool store_signal_rst_port(sc_core::sc_object * port);
        // bool get_current_value_bool(const std::string & port_name);
        // double get_current_value_double(const std::string & port_name);
        // double get_current_value_signal_rst(const std::string & port_name);
        // bool get_checkpoint_value_bool(const std::string & port_name);
        // double get_checkpoint_value_double(const std::string & port_name);
        // double get_checkpoint_value_signal_rst(const std::string & port_name);
        bool get_current_value_bool(long int port_name);
        double get_current_value_double(long int port_name);
        double get_current_value_signal_rst(long int port_name);
        bool get_checkpoint_value_bool(long int port_name);
        double get_checkpoint_value_double(long int port_name);
        double get_checkpoint_value_signal_rst(long int port_name);


};

#endif