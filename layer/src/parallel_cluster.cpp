#include "parallel_cluster.h"


parallel_cluster::parallel_cluster() :
    global_time(0, sc_core::SC_SEC), 
    last_input_event_time(0, sc_core::SC_SEC),
    t_cluster(0, sc_core::SC_SEC),
    estimated_time_between_inputs(0, sc_core::SC_SEC)
{
    avg_processes = 0;
}

bool parallel_cluster::add_module(sct_kernel::ode_system *mod_ptr) {
    
    for (auto ptr: modules) {
        if (ptr == mod_ptr) {
            return false;
        }
    }
    modules.push_back(mod_ptr);
    return true;
}

bool parallel_cluster::remove_module(sct_kernel::ode_system *mod_ptr) {

    for (auto it = modules.begin(); it != modules.end(); it++) {
        if (*it == mod_ptr) {
            modules.erase(it);
            return true;
        }
    }

    return false; 
}

void parallel_cluster::execute() {
    get_global_time();
    estimate_time_between_inputs();

    // Althoug the follwing can be simplified, 
    // we prefer to maintain the four branches in the case 
    // we decide to make a particular decision conditional branch

    if (is_delta_cycle()) {
        // std::cout << "Delta cycle ";
        if (cluster_t_greater_than_global_t()) {
            // std::cout << " had advanced time" << std::endl;
            auto runnable_modules = get_modules_activated_by_input();
            execute_and_synchronize(runnable_modules, t_cluster);
        }
        else {
            // std::cout << "timeout" << std::endl;
            auto t_next = get_next_cluster_time();
            execute_and_synchronize(modules, t_next);
        }

    }
    else {
        // std::cout << "Not delta cycle ";
        if (has_been_activated_by_input() && cluster_t_greater_than_global_t()) {
            // std::cout << " activated by input before t_cluster" << std::endl;
            auto runnable_modules = get_modules_activated_by_input();
            execute_and_synchronize(runnable_modules, t_cluster);
        }
        else {
            // std::cout << " activated at t_cluster exactly" << std::endl;
            auto t_next = get_next_cluster_time();
            execute_and_synchronize(modules, t_next);
        }

    }

    
    last_activation_time = global_time;
    // std::cout << "DIFF time " << t_cluster - last_activation_time << std::endl;
}

void parallel_cluster::estimate_time_between_inputs() {
    
    // for get. the average time between input events
    if (last_input_event_time != global_time) {
        // If it's the first input event, just take its time
        if (estimated_time_between_inputs == sc_core::sc_time(0.0, sc_core::SC_SEC)) {
            estimated_time_between_inputs = global_time;
        }
        else {
            // Take an average between the current activated time 
            // and the time since the last input event.
            auto time_between_inputs = global_time - last_input_event_time;
            estimated_time_between_inputs = 
                (estimated_time_between_inputs + time_between_inputs) / 2.0;
        }
        // Update time of the last input event if 
        // the has been at least one input event at the 
        // current time
        if (has_been_activated_by_input()) {
            last_input_event_time = global_time;
        }
    }

}


sc_core::sc_time parallel_cluster::get_next_cluster_time() {
    
    return t_cluster + STEP_COEFF * estimated_time_between_inputs;
}

sc_core::sc_time parallel_cluster::get_current_cluster_time() {
    
    auto min_time_module_ptr = *(std::min_element(modules.begin(), modules.end(), [](
        sct_kernel::ode_system *mod_ptr_1, 
        sct_kernel::ode_system *mod_ptr_2) {
            return mod_ptr_1->get_local_time() < mod_ptr_2->get_local_time();
        }));

    return min_time_module_ptr->get_local_time();
}

sc_core::sc_time parallel_cluster::get_time() {
    return t_cluster;
}

void parallel_cluster::execute_parallel(
    const module_container &modules_to_execute,
    sc_core::sc_time t_cluster_next) 
{
    // Execute blocks in parallel
   // static tbb::affinity_partitioner ap;
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, modules_to_execute.size()),
        
        [&](const tbb::blocked_range<size_t>& r) {

            for (auto i = r.begin(); i != r.end(); ++i) {    
                sct_kernel::synchronization_layer *sync_layer_ptr;

                sync_layer_ptr = modules_to_execute.operator[](i)->get_sync_layer();

                auto step = t_cluster_next - sync_layer_ptr->get_local_time() ;

                sync_layer_ptr->set_sync_step(step.to_seconds());
                sync_layer_ptr->execute();
            }
        }//, ap
    );    

}

void parallel_cluster::synchronize_modules(sc_core::sc_time t) {

    //static tbb::affinity_partitioner ap;
    tbb::parallel_for(
        tbb::blocked_range<size_t>(0, modules.size()),
        
        [&](const tbb::blocked_range<size_t>& r) {

            for (auto i = r.begin(); i != r.end(); ++i) {    
                modules.operator[](i)->get_sync_layer()->synchronize(t);
            }
        }//, ap
    );    

}

bool parallel_cluster::has_been_activated_by_input() {

    for (auto mod_ptr : modules) {
        if (mod_ptr->get_sync_layer()->has_been_activated_by_input()) { 
            return true;
        }
    }

    return false;
}

void parallel_cluster::get_global_time() {
    global_time = sc_core::sc_time_stamp();
}

void parallel_cluster::generate_outputs() {
    
    // Launch a thread for each process in the set of processes to run
    for (auto mod_ptr : modules) {
        // Outputs should be generated sequentially 
        // to make sure that the values are written in the 
        // communication channels of the main process and not in thread copies
        // of these channels
        if (mod_ptr->should_generate_outputs()) {
            mod_ptr->generate_outputs();
        }
    }
}

// Returns true if it is a delta cycle
// Assumes that the value of global time 
// is up-to-date.
bool parallel_cluster::is_delta_cycle() const {

    if (last_activation_time == global_time) {
        return true;
    }
    return false;
}

bool parallel_cluster::cluster_t_greater_than_global_t() const {
    return t_cluster > global_time; 
}

void parallel_cluster::initialize() {

    auto min_step_mod_ptr = *(std::min_element(modules.begin(), modules.end(), [](
        sct_kernel::ode_system *mod_ptr_1, 
        sct_kernel::ode_system *mod_ptr_2) {
            return mod_ptr_1->get_sync_layer()->get_max_timestep() < mod_ptr_2->get_sync_layer()->get_max_timestep();
        }));

    auto step = min_step_mod_ptr->get_sync_layer()->get_max_timestep();
    estimated_time_between_inputs = sc_core::sc_time(step, sc_core::SC_SEC);
}

parallel_cluster::module_container parallel_cluster::get_modules_activated_by_input() const {
    module_container result;
    for (auto mod_ptr : modules) {
        if (mod_ptr->get_sync_layer()->has_been_activated_by_input()) {
            result.push_back(mod_ptr);
        }
    }
    return result;
}

void parallel_cluster::execute_and_synchronize(const module_container &runnable_modules, sc_core::sc_time t) {

    // std::cout << "Executing " << runnable_modules.size() << std::endl;
    avg_processes = (avg_processes + runnable_modules.size() ) / 2.0; 
    generate_outputs();
    execute_parallel(runnable_modules, t);
    t_cluster = get_current_cluster_time();
    synchronize_modules(t_cluster);

}

double parallel_cluster::get_avg_processes() const {
    return avg_processes;
}