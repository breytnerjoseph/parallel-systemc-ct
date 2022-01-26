#include "../include/synchronization_layer.h"

/////////////////////////////////////////////////////////
// PUBLIC METHODS AND UTILITY FUNCTIONS
/////////////////////////////////////////////////////////
sct_kernel::synchronization_layer::synchronization_layer(ode_system *model_ptr, double max_timestep, bool use_adaptive_sync_step, bool use_time_to_next_event, bool parallel_execution) :
    use_adaptive_sync_step(use_adaptive_sync_step),
    use_time_to_next_event(use_time_to_next_event), 
    parallel_execution(parallel_execution)
 {
    initialize_local_time(max_timestep);
    initialize_solver();
    store_system_model(model_ptr);
    extract_equations_from_system();
    create_synchronization_process();
    initialize_statistics();
}

void sct_kernel::synchronization_layer::store_input_event_sensitivity_list(const sc_core::sc_event_or_list &list) {
    input_event_sensitivity_list = list;
}

int sct_kernel::synchronization_layer::get_backtrack_count() {
    return backtrack_count;
}

/////////////////////////////////////////////////////////
// PRIVATE METHODS AND UTILITY FUNCTIONS
/////////////////////////////////////////////////////////

void sct_kernel::synchronization_layer::synchronization_process() {
    while(true) {
        execute();
    }
}

void sct_kernel::synchronization_layer::execute() {
    // std::cout << std::string( std::string(system_ptr->get_proc_name()) + " @"+ std::to_string(sc_core::sc_time_stamp().to_seconds()) + "\n"); 
    handle_reactivation();
    calculate_solutions();
    schedule_reactivation();
  
}

sct_kernel::synchronization_layer::synchronization_layer() {
    statistics_file->close();
    delete statistics_file;
}

void sct_kernel::synchronization_layer::initialize_local_time(double timestep) {
    local_time = sc_core::sc_time(0.0, sc_core::SC_SEC);
    local_time_backup = sc_core::sc_time(0.0, sc_core::SC_SEC);
    last_call_time = sc_core::sc_delta_count();
    activated_by_input_result = false;
    last_step = -1; 
    sync_step = max_timestep = timestep;
    is_first_activation = true;
    first_registering = true;
}

void sct_kernel::synchronization_layer::initialize_solver() {
    solver = new sct_kernel::numerical_solver();

    state_event_observer =  [&](const sct_core::ct_state &x, const double t) { 
        watch_state_events(x, t);
        // system_ptr->write_trace(t, x);
        return state_event_detected;
    };

    state_event_detected = false;
    state_event_located = false;
    catching_up = false;
}

void sct_kernel::synchronization_layer::extract_equations_from_system() {
    // Lambda function for differential equation (state space)
    diff_equations_func = [&](const sct_core::ct_state &x , sct_core::ct_state &dxdt , double t) { 
        system_ptr->get_derivatives(this->catching_up, x, dxdt, t);
    };

    // Lambda function for threshold detection conditions
    is_event_func = [&](const sct_core::ct_state &x, double t) { 
        // Be aware that we rely on is_event_func storing the events in the
        // last call
        //  std::unordered_map<int, bool> _events =  system_ptr->is_event(x, t);
        // for (auto el : _events) {
        //     if (el.second) {
        //         // Store last set of found events
        //         events = _events;
        //         return true;
        //     }
        // }
        // return false;
        events = system_ptr->is_event(x, t);
        return events;
    };
}

void sct_kernel::synchronization_layer::store_system_model(ode_system *model_ptr) {
    system_ptr = model_ptr;
}

void sct_kernel::synchronization_layer::create_synchronization_process() {
    // Launch the process only in the case of sequential execution
    if (!parallel_execution) {
        sc_core::sc_spawn_options opt;
        sc_core::sc_spawn(
            sc_bind(&synchronization_layer::synchronization_process,this),
            sc_core::sc_gen_unique_name("synchronization_layer_process"),&opt);
    }
    else {
        #ifdef USE_REGISTERING_PROCESS
            sc_core::sc_spawn_options opt;
            sc_core::sc_spawn(
                sc_bind(&synchronization_layer::parallel_registering_process,this),
                sc_core::sc_gen_unique_name("synchronization_layer_process"),&opt);
        #endif
    }
}

void sct_kernel::synchronization_layer::parallel_registering_process() {
    while (true) {
        // Wait for input events and add to list of parallel runnable processes
        // Processes with local_time == to global_time are added automatically 
        // by the parallelization algorithm, we must avoid double inclussion
        get_global_time();
        // if (local_time != global) {
            add_to_parallel_runnable_processes();
        // }
        std::cout << "Exec at " << global_time << std::endl;
        wait(finished_execution_ev);
        wait(local_time - global_time, input_event_sensitivity_list);
        activated_by_input_result = system_ptr->have_inputs_changed();
    }
}

void sct_kernel::synchronization_layer::add_to_parallel_runnable_processes() {
    system_ptr->add_to_parallel_runnable_processes();
}

void sct_kernel::synchronization_layer::handle_reactivation() {

    #ifdef COLLECT_SIM_STATISTICS
        clock_t t = clock();
    #endif
    
    #ifdef DEBUG 
        std::cout << std::endl <<"***************************************" << std::endl <<  "HANDLE-REACTIVATION" << std::endl; 
    #endif 

    // Not strictly necessary as we can always invoke 
    // sc_time_stamp. Just to avoid multiple sc_time_stamp calling cost.
    get_global_time();
    // Collect statistics about input events, not essential 
    // for synchronization, but useful for calc. Delta t. 
    collect_input_event_statistics();
    // Backup state during first delta cycle or restore it if multiple delta cycles



    if (is_delta_cycle() && !is_first_activation) {

        
        if (had_advanced_real_time()) {  
            backtrack();
        }
        else{
            // Outputs are generated to inform of state events
            // or state evolution over the course of delta steps
            if(!parallel_execution) {
                generate_outputs();
            }
            create_backup();                    
        }
    }
    else {
     

        is_first_activation = false;
        // std::cout << "*** NOT A DELTA CYCLE " << std::endl;
        // If process has been activated by input change 
        // the system needs to restore the state (backtrack)
        // Note has_been_activated_by_input() is not necessary here, 
        // according to DATE/TCAD papers. Make tests and remove.
        if (has_been_activated_by_input() && had_advanced_real_time() ) {
            backtrack();
            catch_up();
        }
        // Outputs are generated whenever there is no a delta cycle
        if(!parallel_execution) {
            generate_outputs();
        }
        create_backup();
    }

    #ifdef COLLECT_SIM_STATISTICS
        t = clock() - t;
        t_handle_reactivation += ((double) t) / CLOCKS_PER_SEC;
    #endif
    
    
}


bool sct_kernel::synchronization_layer::had_advanced_real_time() {
    verify_causality();
    // There is no danger if both a time out of the 
    // self invoking wait statement and an input change 
    // schedule the process to be executed at the same time.
    // It is added to set of runnable processes just once. 
    // This is why the following simple condition is sufficient.
    return  global_time < local_time;
}

bool sct_kernel::synchronization_layer::has_been_activated_by_input() {

    #ifdef USE_REGISTERING_PROCESS
        if (parallel_execution) {
            verify_causality();
            return activated_by_input_result;
        }
    #endif

    // Return cached result value to avoid further calculations
    // Notice that sc_delta_count  returns a count of the absolute
    // number of delta cycles that have occurred during simulation
    if (last_call_time == sc_core::sc_delta_count()) {
        return activated_by_input_result;
    }

    // Update last call time
    last_call_time = sc_core::sc_delta_count();

    verify_causality();

    activated_by_input_result = system_ptr->have_inputs_changed();
    
    return activated_by_input_result; 
}


void sct_kernel::synchronization_layer::verify_causality() {
    if (global_time > local_time) {
        std::cout << "*** Causality error in CT/DE synchronization layer: global time (" << 
            global_time << ") > local_time (" << local_time << ")\n";
        /* TODO: use sc_stop and signal stop condition */
        sc_core::sc_stop();
    }
}

void sct_kernel::synchronization_layer::backtrack() {
    #ifdef DEBUG 
        std::cout << "******* BACKTRACKING from " << local_time << " to " << local_time_backup << std::endl;
    #endif

    double backtrack_time = system_ptr->restore_state(global_time.to_seconds());
    local_time = sc_core::sc_time(backtrack_time, sc_core::SC_SEC);
    backtrack_count++;
}

void sct_kernel::synchronization_layer::create_backup() {
    system_ptr->create_checkpoint();
    local_time_backup = local_time;
}


void sct_kernel::synchronization_layer::generate_outputs() {

    if (state_event_located) {
        system_ptr->generate_outputs(
            state_event_located, events
        );
    }
}

void sct_kernel::synchronization_layer::schedule_reactivation() {
    // Schedule its own reactivation only for sequential execution. 
    // In parallel execution, it is the Parallelization-Algorithm 
    // who is responsible of scheduling reactivations
    if (!parallel_execution) {
        sc_core::wait(local_time-global_time, input_event_sensitivity_list);
    }


    #ifdef DEBUG 
        std::cout << "***************************************" << std::endl <<  "SCHEDULE-REACTIVATION" << std::endl; 
        std::cout << "next reactuvation at @t = " << local_time << std::endl << std::endl;
        std::cout << "local time " << local_time << std::endl;
        std::cout << "local time " << global_time << std::endl;
        std::cout << "within " << local_time-global_time << std::endl;
    #endif
    last_activation_time = global_time; 
    
    #ifdef USE_REGISTERING_PROCESS
        activated_by_input_result = false;
    #endif
}

void sct_kernel::synchronization_layer::calculate_solutions() {





    #ifdef DEBUG 
        std::cout << "***************************************" << std::endl <<  "CALCULATE-SOLUTIONS" << std::endl; 
    #endif 

    double t_start = local_time.to_seconds();
    double t_end = get_tentative_end_time();

    
    #ifdef DEBUG 
        std::cout << "t_start" << t_start << std::endl;
        std::cout << "t_end" << t_end << std::endl;
    #endif

    clear_event_flags();
    

    // True when the state has evolved in a delta step or
    // state events have been produced in the delta step 


    bool has_delta_step_dynamics = 
        calculate_delta_step_dynamics(t_start);




    if (has_delta_step_dynamics) {
        t_end = t_start;
        

        system_ptr->write_trace(t_end, system_ptr->get_state());

            
    }
    else{



        #ifdef COLLECT_SIM_STATISTICS
            clock_t t = clock();
        #endif   

        t_end = integrate_equations(t_start, t_end);

        #ifdef COLLECT_SIM_STATISTICS
            t = clock() - t;
            t_calculate_solutions += ((double)t)/CLOCKS_PER_SEC;
        #endif

        t_end = handle_state_events(t_start, t_end);
    }            

    advance_time(t_end);



}

double  sct_kernel::synchronization_layer::get_handle_reactivation_sim_time() const {
    return t_handle_reactivation;
}

double  sct_kernel::synchronization_layer::get_calculate_solutions_sim_time() const {
    return t_calculate_solutions;
}

double  sct_kernel::synchronization_layer::get_t_aux() const {
    return t_aux;
}


bool sct_kernel::synchronization_layer::calculate_delta_step_dynamics(double t_start) {
    
    if (has_been_activated_by_input()) {
        bool instantaneous_state_update = system_ptr->execute_updates();
        bool state_event = 
            locate_state_event_at_start_of_interval(t_start);
        return instantaneous_state_update || state_event;
    }

    return false;
}

double sct_kernel::synchronization_layer::get_tentative_end_time() {
    double step = get_step();

    // We have to be very careful. If there are two CT modules, 
    // it is likely that the system will deadlock.
    if (use_time_to_next_event) {
        double time_to_next_event = sc_core::sc_time_to_pending_activity().to_seconds();

        step = time_to_next_event < step ? time_to_next_event : step;
    }

    // Time of next predictable event in CT
    sc_core::sc_time next_predictable_event_time = system_ptr->get_next_predictable_event_time();

    if (next_predictable_event_time > global_time) {
        double t_to_predictable_ev  = (next_predictable_event_time - global_time).to_seconds();
        step = t_to_predictable_ev < step ? t_to_predictable_ev : step;
    }

    #ifdef DEBUG 
        std::cout << "Using a step of " << step << " seconds" <<  std::endl;
    #endif

    return local_time.to_seconds() + step;
}

double sct_kernel::synchronization_layer::get_step() {

    if (parallel_execution) {
        // std::cout << sc_core::sc_time_stamp().to_seconds() << "\t" "Step " << sync_step << std::endl;
        return sync_step;
    }

    if (last_step == -1) {
        last_step = sync_step;
    }
    
    if (use_adaptive_sync_step) {
        last_step = is_delta_cycle() ? last_step : ADAPTIVE_STEP_AVG_COEFF * estimated_time_between_inputs;
    }
    else {
        last_step = sync_step;
    }

    // std::cout <<  sc_core::sc_time_stamp().to_seconds() << "\t" "Step " << last_step << std::endl;

    #ifdef WRITE_STATISTICS_TO_FILE 
        (*statistics_file) << sc_core::sc_time_stamp().to_seconds() << "\t" << last_step << std::endl;
    #endif

    return last_step;
}

double sct_kernel::synchronization_layer::integrate_equations(double t_start, double t_end) {

    #ifdef DEBUG 
        std::cout << "--- integrate_equations " << std::endl;
        std::cout << "From " << t_start << " to " << t_end << std::endl;
    #endif

    if (t_end > t_start) {
        return solver->integrate(
            diff_equations_func, 
            system_ptr->get_state(),
            t_start,
            t_end, 
            state_event_observer, 
            [&](double t, const sct_core::ct_state &state){ system_ptr->write_trace(t, state);} // tracer
        );                
    }

    return t_end;

    #ifdef DEBUG 
        std::cout << "--- end integration" << std::endl;
    #endif
}

// Advance local time to interval end time or crossing time
void sct_kernel::synchronization_layer::advance_time(double t_end) {
    sc_core::sc_time end_time(t_end, sc_core::SC_SEC);
    local_time = end_time;
}

// Returns threshold crossing time if any in (t_start, t_end])
double sct_kernel::synchronization_layer::handle_state_events(double t_start, double t_end) {
    double t_state_event = t_end; 

    if (state_event_detected && t_max_se > t_min_se) {

        #ifdef DEBUG 
            std::cout << "--- handle_state_events " << std::endl;
            std::cout << "t_min_se " << t_min_se << "t_max_se " << t_max_se << std::endl;
        #endif

        t_state_event = t_max_se;
        sct_core::ct_state state_at_event = x_min_se; 
        state_event_located = true;

        // #ifdef COLLECT_SIM_STATISTICS
        //     clock_t t = clock();
        // #endif

        locate_state_event(t_min_se, t_state_event, state_at_event, x_max_se);

        // #ifdef COLLECT_SIM_STATISTICS
        //     t = clock() - t;
        //     t_aux += ((double)t)/CLOCKS_PER_SEC;
        // #endif
        

        system_ptr->restore_trace(t_min_se);

        system_ptr->set_state(state_at_event);

        system_ptr->write_trace(t_state_event, state_at_event);

        events = true;

        // }
        // else {
        //     t_state_event = t_end;
        //     state_event_located = false;
        // }

        // events = system_ptr->is_event(system_ptr->get_state(), t_max_se);


        #ifdef DEBUG 
            std::cout << "---- end handle_state_events " << std::endl;
        #endif
    }

    return t_state_event;
}

bool sct_kernel::synchronization_layer::locate_state_event_at_start_of_interval(const double t) {
    state_event_located = false;

    // It restricts state events only to the case where 
    // there is a change at the inputs. 
    // If it was due to a change in the state, it would
    // have been detected during the previous integration 
    // procedure. 

    // bool is_event;

    events = system_ptr->is_event(system_ptr->get_state(), t);
    
    if (!events) {
        return false;
    }

    // for (auto el : events) {
    //     if (el.second) {
    //         is_event = true;
    //         break;
    //     }
    // }

    // if (!is_event) {
    //     return false;
    // }
    
    state_event_located = true;

    return state_event_located;
}

void sct_kernel::synchronization_layer::catch_up() {
    catching_up = true;
    clear_event_flags();
    double t_start = local_time.to_seconds();
    double t_end = global_time.to_seconds();
    integrate_equations(t_start, t_end);
    local_time = global_time;
    catching_up = false;
}

void sct_kernel::synchronization_layer::watch_state_events( const sct_core::ct_state &x , const double t )
{
    // Detect events only if not catching up
    // and if event not already detected 
    // and if it is not at the start of the integration
    // interval
    if ( !catching_up && (!state_event_detected) && t > local_time.to_seconds() )
    {
        // Test condition
        state_event_detected = is_event_func(x, t);
        // std::cout << "IS DETECTED " << state_event_detected << std::endl;
        // If event detected, save time
        if (state_event_detected) {
            t_max_se = t;
            x_max_se = x;
        }
        // else, save time of last point where no event has been detected and a copy of the corresponding state
        else {
            t_min_se = t;
            x_min_se = x;

        }
        // t_max_se and t_min_se enclose t_se and are passed to the locate procedure
    }
}

bool sct_kernel::synchronization_layer::is_delta_cycle() {
    if (last_activation_time == global_time) {
        return true;
    }
    
    return false;
}

void sct_kernel::synchronization_layer::get_global_time() {
    global_time = sc_core::sc_time_stamp();
}


void sct_kernel::synchronization_layer::clear_event_flags() {
    state_event_detected = false;
    state_event_located = false;
    events = false; 
    t_min_se = t_max_se = global_time.to_seconds();
    x_min_se = system_ptr->get_backup_state();
}

bool sct_kernel::synchronization_layer::locate_state_event(const double t_start, double &t_end, sct_core::ct_state &state_at_event, const sct_core::ct_state &state_end) {
    bool located = false; 

    located = solver->locate_state_event(
        diff_equations_func,
        state_at_event,
        state_at_event,
        state_end,
        t_start,
        t_end, 
        is_event_func,
        [&](double t, sct_core::ct_state &state){ 
            
            system_ptr->write_trace(t, state);
        } 
    );

    return located;
}

void sct_kernel::synchronization_layer::initialize_statistics() {
    last_input_event_time = sc_core::sc_time(0, sc_core::SC_SEC);
    estimated_time_between_inputs = 0.0;
    backtrack_count = 0;

    t_handle_reactivation = 0;
    t_calculate_solutions = 0;
    t_schedule_reactivation = 0;
    t_aux = 0;

    #ifdef WRITE_STATISTICS_TO_FILE 
        statistics_file = new std::ofstream();
        statistics_file->open(STATISTICS_FILENAME);
    #endif
}

void sct_kernel::synchronization_layer::collect_input_event_statistics () {
    // for calc. the average time between input events
    if (last_input_event_time != global_time) {
        // If it's the first input event, just take its time
        if (estimated_time_between_inputs == 0.0) {
            estimated_time_between_inputs = global_time.to_seconds();
        }
        else {
            // Take an average between the current activated time 
            // and the time since the last input event.
            double time_between_inputs = (global_time - last_input_event_time).to_seconds();
            estimated_time_between_inputs = 
                (estimated_time_between_inputs + time_between_inputs) /2.0;
        }
        // Update time of the last input event if 
        // the has been at least one input event at the 
        // current time
        if (has_been_activated_by_input()) {
            last_input_event_time = global_time;
        }
    }
    
}

const sc_core::sc_time &sct_kernel::synchronization_layer::get_local_time()    const 
{
    return local_time;
}

bool sct_kernel::synchronization_layer::should_generate_outputs()  {
    get_global_time();

    return ( (is_delta_cycle() && !had_advanced_real_time()) || 
        (!is_delta_cycle() && ! (has_been_activated_by_input() && had_advanced_real_time())) );
}

void sct_kernel::synchronization_layer::set_sync_step(double step) {
    sync_step = (step > 0) ? step : sync_step;
} 

double sct_kernel::synchronization_layer::get_sync_step() const {
    return sync_step;
}

void sct_kernel::synchronization_layer::set_adaptive_sync(bool use_adaptive) {
    use_adaptive_sync_step = use_adaptive;
} 

void sct_kernel::synchronization_layer::set_sync_on_next_event(bool sync_on_next_event) {
    use_time_to_next_event = sync_on_next_event;
} 

void sct_kernel::synchronization_layer::synchronize(sc_core::sc_time t) {


    if (t > local_time) {
        std::cout << "Error synchronizing\n";
        sc_core::sc_stop();
    }

    if (t == local_time) {
        return;
    } 

    double new_t = system_ptr->restore_state(t.to_seconds());

    // Catch up if necessary
    if (new_t < t.to_seconds()) {
        catching_up = true;
        clear_event_flags();
        double t_start = new_t;
        double t_end = t.to_seconds();
        integrate_equations(t_start, t_end);
        catching_up = false;
    }

    local_time = t;

}

double sct_kernel::synchronization_layer::get_max_timestep() const {
    return max_timestep;
}