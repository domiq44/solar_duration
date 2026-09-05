// --- File: simulation.h ---

#ifndef SIMULATION_H
#define SIMULATION_H

#include <stddef.h>

#include "data_types.h"

/**
 * @brief Executes the main simulation loop from start date to end date.
 * @param final The finalized configuration.
 */
void run_simulation(const FinalConfig *final);

#endif // SIMULATION_H
