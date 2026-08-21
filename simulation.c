// --- File: simulation.c ---

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date.h"
#include "geo.h"
#include "logger.h"
#include "simulation.h"
#include "solar.h"

// ==========================================================================
// DAY PROCESSING LOGIC
// ==========================================================================

/**
 * @brief Processes a single day: calculates solar duration, converts to H:M:S,
 * and prints the result.
 * @param final La configuration finalisée.
 * @param day Le jour du mois.
 * @param month Le mois.
 * @param year L'année.
 * @param latitude La latitude en double.
 */
static void process_day(const FinalConfig *final, int day, int month, int year,
                        const double latitude) {
  // 1. Date to Ordinal
  int ordinal_target = date_to_ordinal(day, month, year);

  if (ordinal_target == -1) {
    log_error(
        "Date invalide trouvée lors du traitement (%02d/%02d/%d). Ignorée.",
        day, month, year);
    return;
  }

  // 2. Solar Range Check (Based on Ordinal)
  if (ordinal_target < MIN_SOLSTICE_DAY || ordinal_target > MAX_SOLSTICE_DAY) {
    log_warn("Jour ordinal (%d) en dehors de la plage valide (%d-%d). Ignoré.",
             ordinal_target, MIN_SOLSTICE_DAY, MAX_SOLSTICE_DAY);
    return; // Ignorer ce jour
  }

  // Calculate declination ONCE
  double delta_deg = final->declination_func(ordinal_target, year);

  // 3. Calculate duration
  double duration = calculate_day_duration(delta_deg, final->latitude);

  // Convert duration (hours) to H:M:S
  double total_seconds = duration * 3600.0;
  int hours = (int)(total_seconds / 3600.0);
  double remainder_sec = fmod(total_seconds, 3600.0);
  int mins = (int)(remainder_sec / 60.0);
  int secs = (int)round(fmod(remainder_sec, 60.0));

  // 4. Print Results
  log_info("%02d/%02d/%02d (Ord: %02d) | Delta: %.2f° | Lat: %.2f° -> "
           "%02d:%02d:%02d",
           day, month, year, ordinal_target, delta_deg, latitude, hours, mins,
           secs);
}

// ==========================================================================
// SIMULATION LOOP
// ==========================================================================

/**
 * @brief Executes the main simulation loop from start date to end date.
 */
void run_simulation(const FinalConfig *final) {
  log_info("Démarrage de la simulation pour la période définie.");

  // Initialisation des compteurs de boucle
  int current_day = final->jour_debut;
  int current_month = final->mois_debut;
  int current_year = final->annee;

  // Boucle principale de simulation
  while (current_month < final->mois_fin ||
         (current_month == final->mois_fin && current_day <= final->jour_fin)) {

    // 1. Process the current day
    process_day(final, current_day, current_month, current_year,
                final->latitude);

    // 2. Advance in time
    advance_day(&current_day, &current_month, &current_year);
  }

  log_info(
      "La simulation a parcouru toutes les journées jusqu'à la date de fin.");
}
