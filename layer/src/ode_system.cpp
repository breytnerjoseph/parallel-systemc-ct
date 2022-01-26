#include "../include/ode_system.h"

sct_kernel::ode_system::ode_system(){
    // TO PUT IN A CLASS RESPONSIBLE OF LAUNCHING ANYTHING RELATED TO OUR SYNC. ALG. SO THAT THIS GETS EXECUTED JUST ONCE.
    // NOT DANGEROUS TO EXECUTE MULTIPLE TIMES, BUT NOT ELEGANT THOUGH.
    using namespace boost::numeric::odeint;
    // BOOST_STATIC_ASSERT(is_resizeable<sct_core::ct_state>::value == true );
}


// Threshold crossing condition.
// Conceptually, it depends on the inputs. But they are not
// passed because only he current values are needed (not the 
// checkpoint and they are available through the ports)
// It returns an unordered map of pairs (int event_identifier, bool occurred) 
// signaling the conditions that have been met. 
// Identifiers belong to the set
// of integers and their meaning depend on the model.  
// std::unordered_map<int, bool>  sct_kernel::ode_system::is_event(const sct_core::ct_state &x, double t) {
//     return std::unordered_map<int, bool>();
// }

bool sct_kernel::ode_system::is_event(const sct_core::ct_state &x, double t) {
    return false;
}



// Calculates user-defined instantaneous state updates
// based on changes at the inputs 
// (current inputs only, no need to pass them as args)
// (Muxes located exactly after integrators, for example)
// Returns true if an instantaneous update has taken place
// and false otherwise
bool sct_kernel::ode_system::execute_updates() {
    return false;
}

/////////////////////////////////////////////
// GENERAL METHODS
/////////////////////////////////////////////
void sct_kernel::ode_system::create_checkpoint() {
    this->backup_state();
    inputs.create_checkpoint();
}

void sct_kernel::ode_system::backup_state(){
    x_backup = x;
}

double sct_kernel::ode_system::restore_state(double t) {
    // x = x_backup;

    restore_trace(t);

    x = trace.back().second;
        
    return trace.back().first;
}

sct_core::ct_state &sct_kernel::ode_system::get_state(){
    return this->x;
}

bool sct_kernel::ode_system::set_state(sct_core::ct_state x){
    this->x = x;
    return true;
}

const sct_core::ct_state &sct_kernel::ode_system::get_backup_state(){
    return this->x_backup;
}

sc_core::sc_time sct_kernel::ode_system::get_next_predictable_event_time() {
    return sc_core::sc_time(0, sc_core::SC_SEC);
}

// The synch. layer is agnostic about the existence of the inputs 
// manager. This way, MoCs are not obliged to depend on an 
// input manager. But they are obliged to provide the methods 
// on which the synchronization layer depends. 
bool sct_kernel::ode_system::have_inputs_changed() {
    return inputs.have_inputs_changed();
}

// Function to map the state to the traces.
// Default implementation: identity function 
const sct_core::ct_state &sct_kernel::ode_system::map_state_to_trace(double t, const sct_core::ct_state &x) {
    return x;
}

void sct_kernel::ode_system::write_trace(double t, const sct_core::ct_state &x){
    trace_list_node.first = t;
    trace_list_node.second.get_core().noalias() = x.get_core();
    trace.push_back(trace_list_node);
    // trace_map[t] = map_state_to_trace(t, x);
}

void sct_kernel::ode_system::create_trace_file(std::string file_name) {
    std::ofstream trace_file;
    trace_file.open(file_name + ".dat");

    std::string trace_data;

    sct_core::ct_state mapped_state(x.size());

    for (auto [t, state] : trace) {

        trace_data = std::to_string(t);

        mapped_state = map_state_to_trace(t, state);

        for (auto el : state) {
            trace_data += "\t" + std::to_string(el);
        } 

        trace_file << trace_data << std::endl;
    }
}

void sct_kernel::ode_system::restore_trace(double t) {
    sc_core::sc_time t_limit(t, sc_core::SC_SEC);

    while (!trace.empty() && sc_core::sc_time(trace.back().first, sc_core::SC_SEC) > t_limit){
        trace.pop_back();
    }

}

inputs_manager &sct_kernel::ode_system::get_inputs_manager() {
    return inputs;
}