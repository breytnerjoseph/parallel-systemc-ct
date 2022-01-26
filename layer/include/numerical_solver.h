#ifndef NUMERICAL_SOLVER_H
#define NUMERICAL_SOLVER_H

#include <utility>
#include <algorithm>
#include <array>
#include <time.h>
#include <iomanip>
#include "ct_state.h"

///////////////////////////////////////////////////////////////////////
// SOLVER LIBRARY
#include <boost/numeric/odeint/integrate/integrate_adaptive.hpp>
#include <boost/numeric/odeint/stepper/runge_kutta_dopri5.hpp>
#include <boost/numeric/odeint/stepper/generation.hpp>
#include <boost/numeric/odeint/iterator/adaptive_time_iterator.hpp>
#include <boost/numeric/odeint/iterator/adaptive_iterator.hpp>
#include <boost/numeric/odeint/integrate/max_step_checker.hpp>
#include <boost/numeric/odeint/util/detail/less_with_sign.hpp>
#include <boost/numeric/odeint/stepper/controlled_step_result.hpp>
namespace odeint = boost::numeric::odeint;
///////////////////////////////////////////////////////////////////////


// Integration algorithm is adaptive, 
// these values refer just to the first step
#define MAX_STEP 0.1 // seconds
#define MIN_STEP 0.0001 // seconds
 

namespace sct_kernel {

    class numerical_solver {
        public: 
            numerical_solver(){
               dt = -1;
            };

            // Integration method
            template<class DiffEquations, class State, class EventDetector, class Tracer>
            double integrate( DiffEquations diff_equations, State &state, 
                double t_start, double t_end, EventDetector event_detector, Tracer tracer)
            { 
                double int_step = (t_end-t_start) / 10.0;
                // int_step = int_step > MAX_STEP ? MAX_STEP : int_step; 
                int_step = int_step < MIN_STEP ? (t_end-t_start) / 2 : int_step; 
                if (dt == -1) {
                    dt = int_step;
                    hmax = (t_end - t_start) / 3.0;

                    int state_size = state.size();
                    K1.resize(state_size);
                    K2.resize(state_size);
                    K3.resize(state_size);
                    K4.resize(state_size);
                    K5.resize(state_size);
                    K6.resize(state_size);
                    x.resize(state_size);
                    xi.resize(state_size);
                    dxdti.resize(state_size);
                    temp.resize(state_size);
                }

                // a_x and a-dxdt have their default values of 1.
                // Error compared with  eps_abs + eps_rel * ( a_x
                // * |x| + a_dxdt * dt * |dxdt|)
                double abs_err = 1.0e-8, rel_err = 1.0e-8,  a_x = 1.0 , a_dxdt = 1.0;

                #define USE_ODEINT 

                #ifdef USE_ODEINT 

                typedef odeint::runge_kutta_dopri5< State, double, State, double, odeint::vector_space_algebra> error_stepper;

                typedef odeint::default_error_checker<
                    typename error_stepper::value_type ,
                    typename error_stepper::algebra_type ,  
                    typename error_stepper::operations_type>  error_checker;

                typedef odeint::controlled_runge_kutta< error_stepper, error_checker > stepper_type;


                
                auto stepper = stepper_type(error_checker( abs_err , rel_err, a_x, a_dxdt) );

                // to throw a runtime_error if step size adjustment fails
                odeint::failed_step_checker fail_checker;  

                while (odeint::detail::less_with_sign(t_start , t_end , dt)){
                    tracer(t_start, state);
                    if (odeint::detail::less_with_sign(t_end, t_start + dt, dt)) {
                        dt = t_end - t_start;
                    }

                    odeint::controlled_step_result res;
                    do {
                        res = stepper.try_step(diff_equations, state , t_start , dt);

                        // check number of failed steps
                        fail_checker();  
                    }
                    while (res == odeint::fail);
                    // if we reach here, the step was successful -> reset fail checker
                    fail_checker.reset();  

                    // Avoid integrating until the end of the interval
                    // if one state event is found
                    if (event_detector(state , t_start)) {
                        tracer(t_start, state);
                        break;
                    }
                }

                tracer(t_start, state);

                return t_start;

                #else 

                
                // Runge-Kutta Fehlberg
                bool finished = false;
                double t = t_start;
                double h = dt;
                double hmin = 1e-12;
                double t_k = t;

              

                // State K1, K2, K3, K4, K5, K6, x;

                while (!finished) {

                    // Trace state 
                    tracer(t, state);

                    // Get coefficients
                    // K1
                    // State dxdt; 
                    x.get_core().noalias() = state.get_core();
                    t_k = t;
                    // K1 is dxdt 
                    diff_equations(x, K1, t_k);
                    
                    // K2
                    x.get_core().noalias() = state.get_core() + (1.0 / 4.0 * h) * K1.get_core();
                    t_k = t + (1.0 / 4.0) * h;
                    diff_equations(x, K2, t_k);

                    // K3
                    x.get_core().noalias() = state.get_core() + (3.0 / 32.0 * h) * K1.get_core() + (9.0 / 32.0 * h) * K2.get_core();
                    t_k = t + (3.0 / 8.0) * h;
                    diff_equations(x, K3, t_k);

                    // K4
                    x.get_core().noalias() = state.get_core() + (1932.0 / 2197.0 * h) * K1.get_core()  + (-7200.0 / 2197.0 * h) * K2.get_core()  + (7296.0 / 2197.0 * h) * K3.get_core();
                    t_k = t + (12.0 / 13.0) * h;
                    diff_equations(x, K4, t_k);

                    // K5
                    x.get_core().noalias() = state.get_core() + (439.0 / 216.0 * h) * K1.get_core() + (-8.0 * h) * K2.get_core() + (3680.0 / 513.0 * h) * K3.get_core() + (-845.0 / 4104.0 * h) * K4.get_core();
                    t_k = t + h;
                    diff_equations(x, K5, t_k);

                    // K6
                    x.get_core().noalias() = state.get_core() + (-8.0 / 27.0 * h) * K1.get_core() + (2.0 * h) * K2.get_core() + (-3544.0 / 2565.0 * h) * K3.get_core() + (1859.0 / 4104.0 * h) * K4.get_core() + (-11.0 / 40.0 * h) * K5.get_core();
                    t_k = t + (1.0 / 2.0) * h;
                    diff_equations(x, K6, t_k);

                    // Calculate R
                    temp.get_core().noalias() =  
                        (1.0 / 360.0 * h) * K1.get_core() + 
                        (-128.0/4275.0 * h) * K3.get_core() + 
                        (-2197.0 / 75240.0 * h) * K4.get_core() + 
                        (1.0 / 50.0 * h) * K5.get_core() + 
                        (2.0 / 55.0 * h) * K6.get_core();

                    odeint::vector_space_norm_inf< State > norm_struct; 
                    double norm = norm_struct(temp);
                    xi.get_core().noalias() = state.get_core() + 
                        (25.0 / 216.0 * h) * K1.get_core() + 
                        (1408.0 / 2565.0 * h) * K3.get_core() + 
                        (2197.0 / 4104.0 * h) * K4.get_core() + 
                        (-1.0 / 5.0 * h) * K5.get_core();
                    diff_equations(xi, dxdti, t+h);

                    double den =  abs_err + rel_err*(a_x*norm_struct(xi) + a_dxdt*norm_struct(dxdti));

                    double tol = norm / den;

                    double Oe = 4; // Order of error stepper (RK45)
                    double Os = 5;

                    // Step rejected
                    if (tol > 1) {
                        double val1 = 0.9*pow(tol, -1.0 / (Oe-1));
                        double val2 = 0.2; 
                        h = h * ( (val1 > val2) ? val1 : val2); 
                    }
                    // Step accepted
                    else {
                        if (tol < 0.5) {
                            double val1 = 0.9*pow(tol, -1.0 / Os);
                            double val2 = 5; 
                            h = h * ( (val1 < val2) ? val1 : val2); 
                        }

                        t += h;
                        dt = h;
                        state.get_core().noalias() = xi.get_core();

                        // Avoid integrating until the end of the interval
                        // if one state event is found
                        if (event_detector(state , t)) {
                            // std::cout << "@t " << t << state << std::endl;
                            tracer(t, state);
                            finished = true;
                            break; 
                        }

                    }

                    if (t >= t_end) {
                        finished = true;
                    }
                    else if ((t + h) > t_end) {
                        h = t_end - t;
                    }
                    else if (h < hmin) {
                        finished = true;
                        std::cout << h  << "at " << t << " minimum h exceeded\n";
                        exit(EXIT_FAILURE);
                    }

                }

                return t;

                #endif

            }  

            // Root location method
            template<class DiffEquations, class State, class StateEventDetector, class Tracer>
            bool locate_state_event(
                DiffEquations diff_equations, const State &x0, State &x_at_event, const State &x_end,
                const double t_start, double &t_end,
                StateEventDetector is_event, 
                Tracer tracer) 
            {
                const double precision = 1E-7;
                
                // integrates an ODE until some threshold is crossed
                // returns time and state at the point of the threshold crossing
                // if no threshold crossing is found, some time > t_end is returned
   
                auto stepper = odeint::make_dense_output( 1.0e-3, 1.0e-3, odeint::runge_kutta_dopri5< State, double, State, double, odeint::vector_space_algebra>());


                validate_interval(stepper, diff_equations, is_event, tracer, x0, t_start, t_end);
                
                // the dense out stepper now covers the interval where the condition changes
                // improve the solution by bisection
                double t0 = t_start;
                double t1 = t_end;

                double t_m = 0.5 * (t0 + t1);
                State x_m(x0.size());
                State x1 = x_end;


                // TODO: Make sure to return exactly the point where the state event occurs
                // t0 - tm is equal to t1 - tm because tm = 0.5(t0+t1)
                while (std::abs(t0 - t_m) > precision / 2) { 
                    stepper.calc_state(t_m, x_m); // obtain the corresponding state

                    if (is_event(x_m, t_m)){
                        t1 = t_m;  // condition changer lies before midpoint
                        x1 = x_m;
                    }
                    else{
                        // tracer(t_m, x_m);
                        t0 = t_m;  // condition changer lies after midpoint
                    }

                    t_m = 0.5 * (t0 + t1);  // get the mid point time
                }

                t_m = t1;

                if (t_m == t_end){
                    x_m = x_end;
                }
                else{
                    // we find the interval of size eps
                    stepper.calc_state(t_m, x_m);
                }

                t_end = t_m;
                x_at_event = x_m;
 

                // tracer(t_end, x_at_event); 

                return true;
            }


        private:

            template<class Stepper, class DiffEquations, class State, class StateEventDetector, class Tracer> 
            bool validate_interval( Stepper &stepper,
                DiffEquations diff_equations,
                StateEventDetector is_event, Tracer tracer, State x0, 
                const double t_start, double t_end )
            {
                auto ode_range = odeint::make_adaptive_time_range(std::ref(stepper), 
                    diff_equations,
                    x0,
                    t_start,
                    t_end,
                    t_end-t_start/10
                );

                std::find_if(ode_range.first,
                ode_range.second,
                [&](const std::pair<State const&, double> &it){ 
                    State x = it.first; 
                    double t = it.second;
                    return is_event(x, t);
                }); 

                return true;
            }

            // Integration step. Save it to avoid calculating it over and over again
            double dt;
            double hmax;
            sct_core::ct_state K1, K2, K3, K4, K5, K6, x, xi, dxdti, temp;

    };

}

#endif
