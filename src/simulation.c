// --- Fichier : simulation.c ---

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "date.h"
#include "geo.h"
#include "config_validator.h"
#include "logger.h"
#include "simulation.h"
#include "solar.h"

// ==========================================================================
// TRAITEMENT D'UNE JOURNÉE
// ==========================================================================

/**
 * @brief Traite une journée : calcule la durée du jour, la convertit en H:M:S
 * et affiche le résultat.
 * @param final La configuration finalisée.
 * @param day Le jour du mois.
 * @param month Le mois.
 * @param year L'année.
 * @param latitude La latitude en double.
 */
static bool process_day(const FinalConfig *final, CalendarDate date,
                        FILE *csv_output) {
  // 1. Conversion de la date en jour ordinal
  int ordinal_target = date_to_ordinal(date);

  if (ordinal_target == -1) {
    log_error("Date invalide trouvée lors du traitement (%02d/%02d/%d).",
              date.jour, date.mois, date.annee);
    return false;
  }

  // Calculer la déclinaison une seule fois
  SolarDate solar_date = {.jour_ordinal = ordinal_target,
                          .annee = date.annee};
  double delta_deg = final->declination_func(solar_date);

  // 3. Calcul de la durée
  SolarObservation observation = {
      .declinaison_degres = delta_deg,
      .latitude_degres = final->latitude,
      .altitude_solaire_seuil_degres =
          final->altitude_solaire_seuil_degres};
  double duration = calculate_day_duration(observation);

  // Conversion de la durée (heures) au format H:M:S
  int total_seconds = (int)lround(duration * 3600.0);
  int hours = total_seconds / 3600;
  int mins = (total_seconds % 3600) / 60;
  int secs = total_seconds % 60;

  if (csv_output) {
    double duration_sinusoidal = duration;
    double duration_spencer = duration;
    double duration_meeus = duration;
    if (final->mode_declinaison != MODE_SINUSOIDAL) {
      duration_sinusoidal = calculate_day_duration(
          (SolarObservation){
            .declinaison_degres =
              calculate_sinusoidal_declination(solar_date),
            .latitude_degres = final->latitude,
            .altitude_solaire_seuil_degres =
              final->altitude_solaire_seuil_degres});
    }
    if (final->mode_declinaison != MODE_SPENCER) {
      duration_spencer = calculate_day_duration(
          (SolarObservation){
            .declinaison_degres =
              calculate_spencer_declination(solar_date),
            .latitude_degres = final->latitude,
            .altitude_solaire_seuil_degres =
              final->altitude_solaire_seuil_degres});
    }
    if (final->mode_declinaison != MODE_MEEUS) {
      duration_meeus = calculate_day_duration(
          (SolarObservation){
            .declinaison_degres = calculate_meeus_declination(solar_date),
            .latitude_degres = final->latitude,
            .altitude_solaire_seuil_degres =
              final->altitude_solaire_seuil_degres});
    }
    if (fprintf(csv_output,
          "%02d/%02d/%04d,%.8f,%.8f,%.8f,%.8f,%.6f\n", date.jour,
          date.mois, date.annee, duration, duration_sinusoidal,
          duration_spencer, duration_meeus,
          final->altitude_solaire_seuil_degres) < 0) {
      return false;
    }
  }

  // 4. Affichage des résultats
  log_result("%02d/%02d/%02d (Ord: %02d) | Delta: %.2f° | Lat: %.2f° -> "
             "%02d:%02d:%02d",
             date.jour, date.mois, date.annee, ordinal_target, delta_deg,
             final->latitude, hours, mins, secs);
  return true;
}

// ==========================================================================
// BOUCLE DE SIMULATION
// ==========================================================================

/**
 * @brief Exécute la boucle principale de simulation de la date de début à la date de fin.
 */
bool run_simulation(const FinalConfig *final, FILE *csv_output) {
  if (!is_config_fully_valid(final)) {
    return false;
  }

  log_info("Démarrage de la simulation pour la période définie.");

  if (csv_output &&
      fprintf(csv_output,
              "Date,Duration_Hours,Duration_Sinusoidal_Hours,"
          "Duration_Spencer_Hours,Duration_Meeus_Hours,"
          "Solar_Altitude_Threshold_Degrees\n") < 0) {
    log_error("Impossible d'écrire l'en-tête du fichier CSV.");
    return false;
  }

  // Initialisation des compteurs de boucle
  CalendarDate current_date = {final->jour_debut, final->mois_debut,
                               final->annee_debut};

  // Boucle principale de simulation
  bool success = true;
  while (current_date.annee < final->annee_fin ||
        (current_date.annee == final->annee_fin &&
         (current_date.mois < final->mois_fin ||
          (current_date.mois == final->mois_fin &&
           current_date.jour <= final->jour_fin)))) {

    // 1. Traitement de la journée courante
    if (!process_day(final, current_date, csv_output)) {
      success = false;
      break;
    }

    // 2. Avancement de la date
    advance_day(&current_date);
  }

  if (success && csv_output && ferror(csv_output)) {
    success = false;
  }
  if (success) {
    log_info(
        "La simulation a parcouru toutes les journées jusqu'à la date de fin.");
  } else {
    log_error("Échec du traitement de la simulation ou de l'écriture des "
              "résultats.");
  }
  return success;
}
