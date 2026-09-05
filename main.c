// --- File: main.c ---

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_reader.h"
#include "config_validator.h"
#include "data_types.h"
#include "date.h"
#include "geo.h"
#include "logger.h"
#include "simulation.h"
#include "solar.h"

// Prototypes (conservés pour la structure)
static int run_application(const char *config_filename);

int main(int argc, const char *argv[]) {
  // 1. INITIALISATION DU LOGGER
  if (!logger_init("solar_duration.cfg")) {
    // On continue même si le logger n'est pas initialisé, car c'est une erreur
    // de config.
  }

  // 2. Appel de l'orchestrateur principal
  if (!run_application("solar_duration.dat")) {
    return 0;
  } else {
    return 1;
  }
}

// ==========================================================================
// ORCHESTRATEUR PRINCIPAL
// ==========================================================================

static int run_application(const char *config_filename) {

  // --- 1. Instanciation des structures ---
  RawConfig *raw = create_raw_config(); // <-- Utilisation de config_reader
  if (!raw) {
    log_critical(
        "Échec de l'allocation mémoire pour la structure de configuration.");
    return 1;
  }
  FinalConfig final;
  int read_status = READ_SUCCESS;

  log_info("========================================================");
  log_info("--- DÉBUT DU CYCLE DE TRAITEMENT ---");

  // --- 2. LECTURE DES DONNÉES BRUTES (Chaînes) ---
  log_info("--- TENTATIVE DE LECTURE DES DONNEES DANS %s ---", config_filename);
  read_status =
      read_raw_data(config_filename, raw); // <-- Utilisation de config_reader

  if (read_status != READ_SUCCESS) {
    handle_read_error(read_status); // <-- Utilisation de config_validator
    free_raw_config(raw);           // <-- Utilisation de config_reader
    return read_status;
  }

  // --- 3. CONVERSION ET VALIDATION (Chaînes -> Types finaux) ---
  log_info("--- DÉBUT DE LA CONVERSION ET VALIDATION DES DONNEES ---");
  read_status = parse_and_validate_config(
      raw, &final); // <-- Utilisation de config_validator

  // Libérer les données brutes IMMÉDIATEMENT après conversion
  free_raw_config(raw); // <-- Utilisation de config_reader

  if (read_status != READ_SUCCESS) {
    handle_read_error(read_status); // <-- Utilisation de config_validator
    return read_status;
  }

  // --- 4. VALIDATION FINALE (Vérifie les bornes, la structure et la séquence)
  // ---

  // A. Vérification des bornes et de la structure (propriétés)
  if (!is_config_fully_valid(&final)) { // <-- Utilisation de config_validator
    if (log_critical(
            "La configuration est hors bornes ou structurellement invalide."))
      return 1;
    return 1;
  }

  // B. Vérification de la séquence temporelle (cohérence)
  if (!is_time_range_valid(
          final.jour_debut, final.mois_debut, final.jour_fin, final.mois_fin,
          final.annee)) { // <-- Utilisation de config_validator
    if (log_critical("La plage temporelle de début à fin est incohérente."))
      return 1;
  }

  // --- 5. Affichage de la préparation (reste inchangé) ---
  log_info("========================================================");
  log_info("--- SIMULATION PRÊTE À DÉMARRER ---");
  log_info("Période: %02d/%02d/%d au %02d/%02d/%d | Latitude: %.2f",
           final.jour_debut, final.mois_debut, final.annee, final.jour_fin,
           final.mois_fin, final.annee, final.latitude);

  switch (final.mode_declinaison) {
  case MODE_SINUSOIDAL:
    log_info("Utilisation du Mode SINUSOIDAL.");
    break;
  case MODE_SPENCER:
    log_info("Utilisation du Mode SPENCER.");
    break;
  case MODE_MEEUS:
    log_info("Utilisation du Mode MEEUS.");
    break;
  default:
    log_error("Mode de déclinaison inconnu (%d).", final.mode_declinaison);
    return 1;
  }
  log_info("========================================================");
  log_info("");

  // Pré-calcul de la chaîne de latitude formatée (une seule fois)
  char lat_str[50];
  format_latitude(final.latitude, lat_str, sizeof(lat_str));
  log_info("Latitude cible: %s", lat_str);

  // --- 6. EXÉCUTION DE LA SIMULATION ---
  log_info("Démarrage de l'itération de la simulation...");
  run_simulation(&final);

  log_info("========================================================");
  log_info("SIMULATION TERMINÉE AVEC SUCCÈS.");
  log_info("========================================================");

  return 0; // Succès final
}
