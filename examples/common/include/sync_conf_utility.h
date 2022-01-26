#ifndef SYNC_CONF_UTILITY_H
#define SYNC_CONF_UTILITY_H

#include<string>
#include<iostream>

// Takes in the typical argc and argv main parameters 
// and sets the rest of parameters to use
// an adaptive step, to avoid rollbacks, and the 
// timestep, as specified by the user. 
// Returns true if (argc, argv) has a proper formatting,
// (the sync. parameters have been correctly set),  
// and false in any other case (the sync. parameters are not set).  
bool get_sync_conf(int argc, char* argv[],    
    bool &use_adaptive, bool &avoid_rollback, double &timestep, bool &parallel_execution) {
    
    if (argc == 2) {
        parallel_execution = std::string(argv[1]) == std::string("y");
    }

    if (argc != 4) {
        return false;
    }
    else {
        use_adaptive = std::string(argv[1]) == std::string("y");
        avoid_rollback = std::string(argv[2]) == std::string("y");
        timestep = std::stof(argv[3]); 
    }

    return true;
}

bool get_sync_conf(int argc, char* argv[],    
    bool &use_adaptive, bool &avoid_rollback, double &timestep) {
    
    if (argc != 4) {
        return false;
    }
    else {
        use_adaptive = std::string(argv[1]) == std::string("y");
        avoid_rollback = std::string(argv[2]) == std::string("y");
        timestep = std::stof(argv[3]); 
    }

    return true;
}

void print_sync_stats(bool use_adaptive, bool avoid_rollback,
    double timestep,
    double simulated_time, double wallclock_time) 
{ 
    std::cout << "SYNC. STRATEGY: " << (use_adaptive ? "ADAPTIVE" : "FIXED STEP") << std::endl;

    if(!use_adaptive) {
        std::cout << "TIMESTEP: " << timestep << " seconds" << std::endl;
    }

    std::cout << "AVOID ROLLBACK (USE NEXT EVENT'S TIME): " << (avoid_rollback ? "YES" : "NO") << std::endl;

    std::cout << "SIMULATED TIME: " << simulated_time << " seconds" << std::endl;

    std::cout << "WALL-CLOCK SIMULATION TIME: " << wallclock_time << " seconds" << std::endl << std::endl;
}

void print_sync_stats(bool parallel_execution, bool use_adaptive, bool avoid_rollback,
    double timestep,
    double simulated_time, double wallclock_time) 
{ 
    std::cout << "PARALLEL EXECUTION: " << (parallel_execution ? "TRUE" : "FALSE") << std::endl;

    std::cout << "SYNC. STRATEGY: " << (use_adaptive ? "ADAPTIVE" : "FIXED STEP") << std::endl;

    if(!use_adaptive) {
        std::cout << "TIMESTEP: " << timestep << " seconds" << std::endl;
    }

    std::cout << "AVOID ROLLBACK (USE NEXT EVENT'S TIME): " << (avoid_rollback ? "YES" : "NO") << std::endl;

    std::cout << "SIMULATED TIME: " << simulated_time << " seconds" << std::endl;

    std::cout << "WALL-CLOCK SIMULATION TIME: " << wallclock_time << " seconds" << std::endl << std::endl;
}

#endif