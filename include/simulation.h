// --- Fichier : simulation.h ---

#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdbool.h>
#include <stdio.h>

#include "data_types.h"

/**
 * @brief Exécute la simulation de la date de début à la date de fin.
 * @param final La configuration validée.
 * @param csv_output Flux facultatif recevant l'en-tête CSV et les résultats quotidiens.
 * @return true si la configuration est valide et si la simulation ainsi que
 *         l'écriture CSV facultative réussissent.
 */
bool run_simulation(const FinalConfig *final, FILE *csv_output);

#endif // SIMULATION_H
