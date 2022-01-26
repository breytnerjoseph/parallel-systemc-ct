#include "../include/ct_module.h"

sct_core::ct_module::ct_module(
    sc_core::sc_module_name name,
    bool use_adaptive_sync_step, bool use_time_to_next_event, bool parallel_execution) : 
    sc_core::sc_module(name),
    use_adaptive_sync_step(use_adaptive_sync_step),
    use_time_to_next_event(use_time_to_next_event),
    parallel_execution(parallel_execution)    
{ 
}

void sct_core::ct_module::end_of_elaboration(){
    // Invoke method to set attributes as defined by the user
    set_sync_parameters();

    // Register input ports in inputs manager
    sct_kernel::ode_system::inputs.register_input_ports(sc_core::sc_module::get_child_objects());

    // Create synchronization layer object
    this->sync_layer = new sct_kernel::synchronization_layer(this, max_step, 
        use_adaptive_sync_step, use_time_to_next_event, parallel_execution);

    sync_layer->store_input_event_sensitivity_list(
        sct_kernel::ode_system::inputs.get_sensitivity_list()
    );


    // Set initial conditions (as defined by the user)
    // and create a checkpoint on them
    set_initial_conditions();
    sct_kernel::ode_system::backup_state();

    // Use the parallelization algorithm only if marked 
    // for parallel execution
    if (parallel_execution) {
        sct_kernel::parallelization_algorithm::get().store_module(this);
    }
}

void sct_core::ct_module::start_of_simulation() {
    if (parallel_execution) {
        sct_kernel::parallelization_algorithm::get().start_of_simulation();
    }
}

bool sct_core::ct_module::should_generate_outputs() {
    return this->sync_layer->should_generate_outputs();
}

void sct_core::ct_module::generate_outputs() {
    return this->sync_layer->generate_outputs();
}

// To be implemented by the user, 
// as in TDF modules of SystemC AMS
// e.g. call set_max_timestep
// The default behavior is to set 
// a default maximum timestep
void sct_core::ct_module::set_sync_parameters() {
    // Set default maximum synchronization timestep
    set_max_timestep(DEFAULT_DELTA_T);
}

// Set the maximum allowed synchronization timestep
void sct_core::ct_module::set_max_timestep(double max){
    if(max > MIN_DELTA_T){
        max_step = max;
    }
    else{
        max_step = MIN_DELTA_T;
        std::cout << "Couldn't set DELTA T below " << max_step << " s. Set to " << max_step << std::endl; 
    }
}

void sct_core::ct_module::set_initial_conditions() {
    for (auto&& x_val : sct_kernel::ode_system::x) {
        x_val = 0;
    }
}

// 
void sct_core::ct_module::end_of_simulation() {
    // // Clear screen in UNIX systems
    // std::cout << "\x1B[2J\x1B[H"; 
    // std::cout << "********************************************** " << std::endl;
    // std::cout << "* STATISTICS                                 *" << std::endl;
    // std::cout << "********************************************** " << std::endl;
    // std::cout << "NUMBER OF BACKTRACKS : " << sync_layer->get_backtrack_count() << std::endl;
    // std::cout << "TIME SPENT IN HANDLE_REACTIVATION : " << sync_layer->get_handle_reactivation_sim_time() << std::endl;
    // std::cout << "TIME SPENT IN CALCULATE_SOLUTIONS : " << sync_layer->get_calculate_solutions_sim_time() << std::endl;
    

    // if (parallel_execution) {
    //     sct_kernel::parallelization_algorithm::get().print_exec_time();
    // }

    sct_kernel::parallelization_algorithm::get().end_of_simulation();

    // create_trace_file(name());
}

void sct_core::ct_module::add_to_parallel_runnable_processes() {
    sct_kernel::parallelization_algorithm::get().add_to_parallel_runnable_processes(this);
}

void sct_core::ct_module::use_adaptive_sync(bool val){
    use_adaptive_sync_step = val;
}

void sct_core::ct_module::notify_finished_exec() {
    sync_layer->notify_finished_exec();
}

void sct_core::ct_module::avoid_rollbacks(bool val){
    use_time_to_next_event = val;
}

void sct_core::ct_module::execute() {
    sync_layer->execute();
}

const sc_core::sc_time &sct_core::ct_module::get_local_time() const {
    return sync_layer->get_local_time();
}

const char *sct_core::ct_module::get_proc_name() const {
    return name();
}

sct_kernel::synchronization_layer *sct_core::ct_module::get_sync_layer() {
    return sync_layer;
}