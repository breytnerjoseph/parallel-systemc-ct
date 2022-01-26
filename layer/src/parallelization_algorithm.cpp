#include "parallelization_algorithm.h"

using namespace sct_kernel;

// Instantiation of the manager
parallelization_algorithm::manager parallelization_algorithm::my_manager;

parallelization_algorithm::parallelization_algorithm()
{
    launched = false;
    initialization_phase = true;
    avg_proc = 0;

    #ifndef USE_TBB
        pool = new thread_pool;
    #endif
}

parallelization_algorithm &parallelization_algorithm::get() noexcept {
    return *(my_manager.get());
}

parallelization_algorithm::manager::manager() {
    singleton = new parallelization_algorithm;
}

parallelization_algorithm::manager::~manager() noexcept {
    delete singleton;
}

parallelization_algorithm *parallelization_algorithm::manager::get() {
    return singleton;
}

void parallelization_algorithm::store_module(
   sct_kernel::ode_system *module_ptr) 
{
    ode_systems.push_back(module_ptr);
}

void parallelization_algorithm::start_of_simulation() {
    unsigned number_of_processes = ode_systems.size();

    // If the spawn process has not already been called
    bool one_or_more_processes = number_of_processes > 0;
    if (!launched && one_or_more_processes) {
        #ifndef USE_TBB 
            pool->launch_threads(number_of_processes);
        #endif 
            
        #ifndef USE_REGISTERING_PROCESS
            create_sensitivity_list();
        #endif
        
        create_systemc_process();
        initialize_parallel_clusters();
    }
}

void parallelization_algorithm::parallelization_process() {

    #ifndef USE_REGISTERING_PROCESS
        cluster.execute();
        schedule_reactivation();
        while (true) {
            cluster.execute();
            schedule_reactivation();
        }
    #else 
        while (true) {
            execute_processes(processes_to_run);
            for (auto process_ptr : processes_to_run) {
                process_ptr->notify_finished_exec();            
            }
            processes_to_run.clear();
            schedule_reactivation();
        }
    #endif 

}

void parallelization_algorithm::create_systemc_process() {
    sc_core::sc_spawn_options opt;
    sc_core::sc_spawn(
        sc_bind(&parallelization_algorithm::parallelization_process,this),
        sc_core::sc_gen_unique_name("parallelization_process"),&opt);
    launched = true;
}

void parallelization_algorithm::set_static_sensitivity(sc_core::sc_spawn_options &opt) {

    // // Traverse all registered ct modules
    // for (auto system_ptr: ode_systems) {
    //     // For each ct module, get the list of input events 
    //     for (auto event_ptr: system_ptr->get_inputs_manager().get_event_list()) {
    //         // Add the event to the static sensitivity list
    //         opt.set_sensitivity(event_ptr);
    //     }
    // }

    opt.set_sensitivity(&added_new_process_ev);
}

void parallelization_algorithm::create_sensitivity_list() {
    for (auto system_ptr: ode_systems) {
        input_sensitivity_list |= system_ptr->get_inputs_manager().get_sensitivity_list();
    }
}


void get_future(std::future<void> &future) {
    future.get();
}

void parallelization_algorithm::execute_processes(ode_system_container processes_to_run) {

    avg_proc = (avg_proc + processes_to_run.size() ) / 2.0;
 

    // // Launch a thread for each process in the set of processes to run
    // for (auto process_ptr : processes_to_run) {
    //     // Outputs should be generated sequentially 
    //     // to make sure that the values are written in the 
    //     // communication channels of the main process and not in thread copies
    //     // of these channels
    //     if (process_ptr->should_generate_outputs()) {
    //         process_ptr->generate_outputs();
    //     }
    // }

    // cluster.generate_outputs();

    // PARALLEL OUTPUT GENERATION
    // tbb::parallel_for(
    //         tbb::blocked_range<size_t>(0, processes_to_run.size()),
    //         OutputGenerator(&processes_to_run));


    #ifdef USE_TBB 
        // Use affinity partitioner 
        // " It not only automatically chooses the grainsize, but also optimizes for cache affinity and tries to distribute the data uniformly among thread" 
        // (https://software.intel.com/content/www/us/en/develop/documentation/tbb-documentation/top/intel-threading-building-blocks-developer-guide/parallelizing-simple-loops/parallelfor/bandwidth-and-cache-affinity.html)

        // static tbb::affinity_partitioner ap;
        
        // tbb::parallel_for(
        //     tbb::blocked_range<size_t>(0, processes_to_run.size()),
            
        //     [&](const tbb::blocked_range<size_t>& r) {

        //         for (auto i = r.begin(); i != r.end(); ++i) {    
        //             // processes->operator[](i)->get_sync_layer()->set_sync_step(sync_step);

        //             processes_to_run.operator[](i)->get_sync_layer()->execute();
        //         }
        //     },

        //     ap
        // );     

        cluster.execute();


    #else 
            std::vector<std::future<void> > futures(processes_to_run.size());
            unsigned i;
            for (auto process_ptr : processes_to_run) {
                
                // // Submit a function to the pool->
                futures[i] = pool->submit(
                    [=] () {
                        process_ptr->execute();
                    }
                ); 

                i++;
            }

            pool->dispatch();

            //     // Wait for all tasks in the pool to complete.
            for ( unsigned j = 0; j < futures.size(); j++) {
                futures[j].get();
            }
    #endif



}


void parallelization_algorithm::schedule_reactivation() {   
    
    #ifndef USE_REGISTERING_PROCESS
        sc_core::sc_time next_time = next_reactivation_time();
        sc_core::wait(next_time-sc_core::sc_time_stamp(), input_sensitivity_list);
    #else 
        sc_core::wait(added_new_process_ev);
    #endif
    
}

sc_core::sc_time parallelization_algorithm::next_reactivation_time() {
    // sc_core::sc_time minimum_time = (*ode_systems.begin())->get_local_time();

    // for (auto it = ++ode_systems.begin(); it != ode_systems.end(); it++) {
    //     if ( (*it)->get_local_time() < minimum_time) {
    //         minimum_time = (*it)->get_local_time();
    //     }
    // }

//    return  minimum_time;

    return cluster.get_time();
}

void parallelization_algorithm::add_to_parallel_runnable_processes(sct_kernel::ode_system *ode_system) {
    #ifdef USE_REGISTERING_PROCESS
        processes_to_run.push_back(ode_system);
        
        // Schedule a reactivation at the next delta cycle
        // std::cout << "REGISTERING " << ode_system->get_proc_name() << std::endl;
        if (processes_to_run.size() == 1) {
            // added_new_process_ev.notify(sc_core::SC_ZERO_TIME);
            added_new_process_ev.notify(); // WHY ? 
        }
    #endif
}

void parallelization_algorithm::get_processes_to_run() {
    auto global_time = sc_core::sc_time_stamp();

    for (auto system_ptr : ode_systems) {
        if ( system_ptr->get_local_time() == global_time || 
            system_ptr->get_inputs_manager().have_inputs_changed()) {
                processes_to_run.push_back(system_ptr);
        }
    }

}

void parallelization_algorithm::print_exec_time() {
}

void parallelization_algorithm::end_of_simulation() {

    static bool printed_info = false;

    if (!printed_info) {
        std::cout << "Avg proc " << cluster.get_avg_processes()  << std::endl;
        printed_info = true;
    }

    #ifndef USE_TBB
        if (pool) {
            delete pool;
            pool = NULL;
        }
    #endif
}



void parallelization_algorithm::initialize_parallel_clusters() {
    for (auto ptr : ode_systems) {
        cluster.add_module(ptr);
    }
    cluster.initialize();
}