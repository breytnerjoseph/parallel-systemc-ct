------------------------------------------------------------------------------

    README
    SystemC CT: SystemC Continuous Time Library

    Breytner FERNANDEZ <Breytner.Fernandez@univ-grenoble-alpes.fr>
    Liliana ANDRADE <Liliana.Andrade@univ-grenoble-alpes.fr>
    Frédéric Pétrot <Frederic.Petrot@univ-grenoble-alpes.fr>

------------------------------------------------------------------------------

Copyright (C) 2018-2021
Univ. Grenoble Alpes, CNRS, Grenoble INP*, TIMA, 38000 Grenoble, France.
*Institute of Engineering Univ. Grenoble Alpes

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

------------------------------------------------------------------------------

# Parallel SystemC CT 

## Description:
This repository contains a proof-of-concept parallel implementation of the algorithms described in the articles 
*[Synchronization of Continuous Time and Discrete Events Simulation in SystemC](https://ieeexplore.ieee.org/document/9177033)* [1], published in the IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems journal, and 
*[Accurate and Efficient Continuous Time and Discrete Events Simulation in SystemC](https://ieeexplore.ieee.org/document/9116300/)* [2], published in the Design Automation and Test in Europe 2020 conference.

We provide a library of classes that adds modeling and simulation capabilities for continuous time (CT) systems to the discrete events (DE) SystemC simulator. 
A CT system is a system modeling a physical process (electrical, mechanical, chemical,etc.) in the form of differential equations. 
<!-- Consider the example of a switched resistor-capacitor circuit in the following figure:

![rc circuit schematic](./doc/img/rc_circuit.png)

In this circuit, the capacitor voltage evolves with time. 
The differential equations modeling it are 
1. When the switch is closed:
![rc circuit schematic](./doc/img/circuit_eq_1.png)
2. When the switch is open: 
![rc circuit schematic](./doc/img/circuit_eq_2.png)

The switch state can be specified and modified by a switch controller modeled by a regular DE SystemC module. 
In general, CT and DE systems can be interconnected.
Hence the importance of adding CT modeling and simulation capabilities to the existing SystemC simulator.  -->

SystemC CT provides the modeling mechanisms to allow users to specify SystemC modules that: 
1. Have a state evolution in time that is desribed by differential equations. 
2. React to DE inputs that change the differential equations and produce discontinuities in the state. 
3. Detect events produced when the state and time meet some user-specified condition (state events).
4. Generate outputs to inform other CT and DE SystemC modules of these state events.

We describe these mechanisms below and we illustrate them with several ready-to-run examples in this repository. 

SystemC CT is compatible and runs on top of the standard SystemC kernel without modifications. 

Please note that this implementation is a proof of concept and it is neither intended to serve as a reference implementation nor it is suitable for direct use in industrial applications. Its only aim is to support the claim that the algorithms described in our paper are feasible to implement.


## Dependencies:
1. [SystemC 2.3.1a.](https://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.1a.tar.gz)
2. Boost libraries (version 1.71), to install on debian-based systems execute `sudo apt-get install libboost-all-dev`
3. [Intel TBB Library](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html) 
4. [Eigen libraries (unstable version)] (https://gitlab.com/libeigen/eigen/-/archive/master/eigen-master.tar.bz2)
5. (Optional, for comparison purposes) [SystemC AMS](https://www.coseda-tech.com/systemc-ams-proof-of-concept) compiled against SystemC 2.3.1a. 
6. (Optional, for comparison purposes) MATLAB/Simulink or GNU Octave.

## Installation:

Follow the instructions in the INSTALL file.


## Modeling CT modules:

We illustrate modeling of a CT module by a bouncing ball example, which is a commonly used example of a CT/DE system. 

### System Description 
A ball experiences a free fall from a given height (h > 0 m) influenced by the acceleration of earth gravity (g = −9.81 m/s^2 ). 
The following equation describes this dynamics: 

* dh/dt = v

* dv/dt = -g

where h is the ball's height or vertical position, v is the ball's velocity, and g is the gravitational constant. 

Eventually the ball collides with the floor and bounces. 
Collisions can be detected when the ball is falling and its vertical position or height is close to zero: 

* collision_event = v < 0 and h < h_min   

At each collision event, the speed discontinuously goes from a negative value (falling) to a positive one (rising). 

* v = Cr * v, with 0 <= Cr <= 1

Bounces are non-conservative: the ball loses energy (Cr factor) and ball rises to a smaller height at each bounce; the process repeats until the ball loses all of its energy.
We call this event the low energy event because the ball has lost all its potential energy (h is low) as well as its kinetic energy (|v| is low). The following equations define the event:

* low_energy_event = h < h_min and |v| < v_min

After meeting this conditions, the ball stops bouncing.  The following equation models the stopped ball: 

* dh/dt = 0

* dv/dt = 0

The system has two DE states: Bouncing and Stopped.

### Modeling 

We can represent the CT system more formally in four parts: the differential equations, the state events, the state updates, and the output generation function. We can model the two DE states of the ball by a DE module. 

The following figure shows a model of the system. 

![bouncing ball](./doc/img/bouncing_ball_model.png)

We can implement this model in SystemC CT. 
The model is available in `examples/bouncing_ball/ct_de/ball_ct.h` and it is ready to execute as part of a simulation in `examples/bouncing_ball/ct_de/main.cpp`. 
We describe the most relevant steps in building such a model: 
1. Include the SystemC and SystemC CT headers.

```c++
#include <systemc>
#include <systemc-ct.h>
```

2. Declare the BallCt class, which is the CT Ball module that inherits from the sct_core::ct_module class.

```c++
class BallCt : public sct_core::ct_module { 
```

3. Declare the input and output ports, which are regular SystemC boolean and real ports: 

```c++
    public:

        //////////////////////////////////////////////////////////////
        // PORTS 
        //////////////////////////////////////////////////////////////

        // Inputs
        //// Each transition signals a collision and, consequently, 
        //// that the ball should bounce
        sc_core::sc_in<bool> bounce_in;
        //// A transition from false to true means that the ball goes 
        //// from Bouncing to Stopped
        sc_core::sc_in<bool> stop_in;


        // Outputs 
        //// Each transition signals a collision event
        sc_core::sc_out<bool> collision_out;
        //// The first transition from false to true signals
        //// the low energy event
        sc_core::sc_out<bool> low_energy_out;
```


4. Set the size and initial conditions of the CT state by defining the `set_initial_conditions` method:

```c++
        //////////////////////////////////////////////////////////////
        // CT State (x attribute) setting 
        // x is a vector of size equal to the order of the 
        // differential equation (ODE) system. 
        //////////////////////////////////////////////////////////////
        void set_initial_conditions(){
            x.resize(2);
            x[0] = 10; // Position
            x[1] = 0; // Speed
        }
```

5. Set the differential equations by declaring the `get_derivatives` method:
```c++

        //////////////////////////////////////////////////////////////
        // GET-DERIVATIVES
        //////////////////////////////////////////////////////////////
        
        // Given a boolean indicating whether or not to use the checkpointed 
        // value of inputs (use_input_checkpoints), the state (x), 
        // and the time (t), calculate and set the derivatives of the state 
        // (dxdt).  
        // Mandatory
        void get_derivatives(bool use_input_checkpoints,
            const sct_core::ct_state &x,
            sct_core::ct_state &dxdt,
            double t)
        {            
            // Set the mode of the inputs manager to either current_values or checkpoint
            ode_system::inputs.use_checkpoints(use_input_checkpoints);
            // Get the value from the stop_in port
            bool stopped = ode_system::inputs[stop_in];
            
            // Bouncing.
            if (!stopped) { 
                dxdt[0] = x[1]; 
                dxdt[1] = -g;  
            }
            // Stopped, all derivatives to zero.
            else {
                dxdt[0] = 0; 
                dxdt[1] = 0;   
            }
        }

```

6. Set the event detection conditions by defining the `is_event` method:

```c++

        //////////////////////////////////////////////////////////////
        // IS-EVENT
        //////////////////////////////////////////////////////////////
        
        // Given the state (x) and time (t), return an unordered map object 
        // containing elements whose key is an integer identifying the event 
        // and as value a boolean that indicates whether or not 
        // the event has been produced  
        // Not mandatory
        std::unordered_map<int, bool> is_event(const sct_core::ct_state &x, double t = 0){
            std::unordered_map<int, bool> events;

            bool stopped = ode_system::inputs[stop_in];
            
            // Bouncing.
            if (!stopped) { 
                // Condition for the bounce event
                events[collision_event] =  x[0] <= min_x_threshold && x[1] < 0;
                
                events[low_energy_event] = x[0] <= min_x_threshold && fabs(x[1]) < min_v_threshold;
            }

            return events;
        }

```

7. Set the state update conditions by defining the `execute-updates` method: 

```c++
        //////////////////////////////////////////////////////////////
        // EXECUTE-UPDATES
        //////////////////////////////////////////////////////////////
        
        // Execute discontinuous updates in the state depending 
        // on the current inputs, state, and simulation time. 
        // Return true if at least one update. 
        // Not mandatory.
        bool execute_updates() {
            // Condition for the state update
            // Notice that last_bounce_in_value should not constitute
            // a DE state inside the CT module. The problem here is with 
            // the channel : we are not allowed to consume the event 
            // of bouncing, so we need to base our login on the transition 
            // of the boolean variable. This has to be solved with the 
            // definition and implementation of appropriate channels. 
            if(bounce_in.read() != last_bounce_in_value){
                last_bounce_in_value = bounce_in.read();
                // Discontinuity in state
                x[1] = -elasticity_factor*x[1];
                // Return true to indicate the discontinuous change 
                // to the synch. algorithm
                return true;
            }
            // No update
            return false;
        }

```

8. Set the output port values by defining the `generate_outputs` method: 
```c++
        //////////////////////////////////////////////////////////////
        // EXECUTE-UPDATES
        //////////////////////////////////////////////////////////////

        // Given a boolean indicating whether the output events have been 
        // precisely located and an unordered map containing these events, 
        // Write the events in the output ports 
        // Mandatory
        void generate_outputs(bool state_event_located,
            std::unordered_map<int, bool> events) 
        {
            h_out.write(x[0]);
            v_out.write(x[1]);

            // If it is a low energy event
            if (events[low_energy_event]) {
                low_energy_out.write(true);
            }

            // If it is a collision event and it has been located
            if (state_event_located && events[collision_event]) {
                collision_out.write(!collision_out.read());
            }
        }
```

9. Define the class constructor and the class attributes: 
```c++
        
        //////////////////////////////////////////////////////////////
        // Constructor
        //////////////////////////////////////////////////////////////
        BallCt(sc_core::sc_module_name name, 
            bool use_adaptive = true, bool avoid_rollback = true, 
            double sync_step = DELTA_T_BALL_MODEL) 
            : 
            // Call port constructors
            bounce_in("bounce_in"),
            stop_in("stop_in"),
            h_out("h_out"), 
            v_out("v_out"),
            collision_out("collision_out"),
            low_energy_out("low_energy_out"),
            // etc ...
        {
        }

private:
        // System parameters
        double g = 9.81;
        double min_x_threshold = 0.001;
        double min_v_threshold = 0.001;
        double elasticity_factor = 0.8;

        // etc ...
};
```


What is important to notice here is that the methods `set_initial_conditions`, `get_derivatives`, `is_event`, `execute_updates`, and `generate_outputs` of all classes derived from `sct_core::ct_module` are automatically called by the SystemC CT simulation kernel. 
And that their signature is general to all CT modules. 
In other words, to specify a CT module in SystemC CT the user is only required to define these methods.


## Execution:

In the `ct_de` folders of each example:


3. Execute `make` to compile:
```console
foo@bar:~$ make 
```

3. Simulate:
```console
foo@bar:~$ ./main
```

4. (Optional) Run the Octave script to display the results:
```console
foo@bar:~$ octave results.m 
```
5. (Optional) Set the SYSTEMC_AMS_PREFIX environment variable if you want to execute the TDF examples in the `tdf` folder. 


5. (Optional) For the MATLAB/Simulink model of the Vehicle's Automatic Transmission Control example, the script `examples/vehicle_cruise_control/matlab/parameter_values.m` has to be run before executing Simulink. Its purpose is to load the model's parameter values. 

## Models of Computation:
We provide two models of computation (sets of primitives that define the computation and communication rules for modeling systems from a specific domain): Signal Flow, for modeling linear and non-linear CT systems in the form of block diagrams, and Ideally Switched Circuits, for modeling power systems in the form of electrical circuits. 

The Ideally Switched Circuits Model of Computation is completely described in the article *[Simulation of Ideally Switched Circuits in SystemC]( https://www.doi.org/10.1145/3394885.3431417)* [3], published in the proceedings of the IEEE Asia and South Pacific Design Automation Conference (ASP-DAC 2021).

We provide the following modeling examples: 
1. Signal Flow: Switched RC Circuit. 
2. Ideally Switched Circuit: Boost converter, Buck converter, Cuk converter, Flyback converter, SEPI converter. 


## Other Ready-to-run Examples: 

We provide the following examples that test various challenging CT/DE simulation simulation issues (see article [1]): 
1. Bouncing Ball.
2. Switched RC Circuit.
3. Vehicle Cruise Control.
6. Broman tests: integrating discontinuities, integrating glitches, zero delay feedback. 
7. Non holonomic integrator. 

Users can start their models by taking these examples as starting points. 


This is the SystemC CT library.

For missing features or bugs, please send an e-mail to:
    Breytner.Fernandez@univ-grenoble-alpes.fr
    Liliana.Andrade@univ-grenoble-alpes.fr
    Frederic.Petrot@univ-grenoble-alpes.fr

------------------------------------------------------------------------------
