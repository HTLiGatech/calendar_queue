//
//  Sample Discrete Event Simulation
//
//  Created by Richard Fujimoto on 9/24/17.
//  Copyright 2017 Richard Fujimoto. All rights reserved.
//

//
// Application Independent Simulation Engine Interface
//
//
// Function defined in the simulation engine called by the simulation application
//

// Call this procedure to run the simulation indicating time to end simulation
void RunSim (double EndTime, FILE* ofp);

// Schedule an event with timestamp ts, event parameters *data
void Schedule (double ts, void *data);

// This function returns the current simulation time
double CurrentTime (void);

//Initialize the array and other attributes in the Future Event List
void Initialize();

//Exponential distribution random number generator
double urand();
double randexp(double u);
 
//
// Function defined in the simulation application called by the simulation engine
//
//  Event handler function: called to process an event
void EventHandler (void *data, FILE* ofp);
