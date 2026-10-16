// --- Fichier : main.c ---

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
typedef struct {
  const char *config_filename;
  const char *csv_filename;
  int check_only;
} ApplicationFiles;

static int run_application(const ApplicationFiles *files);
static void handle_read_error(const char *config_filename, int read_status);

static void print_usage(const char *program_name) {
  fprintf(stderr,
      "Usage: %s [--help] [--check] [--config FILE] [--logger-config FILE] "
          "[--csv FILE]\n",
          program_name);
}

int main(int argc, const char *argv[]) {
  const char *config_filename = "config/solar_duration.dat";
  const char *logger_filename = "config/solar_duration.cfg";
  const char *csv_filename = NULL;
  int check_only = 0;

  for (int arg_index = 1; arg_index < argc; arg_index++) {
    if (strcmp(argv[arg_index], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    }
    if (strcmp(argv[arg_index], "--check") == 0) {
      check_only = 1;
      continue;
    }
    if (arg_index + 1 >= argc) {
      print_usage(argv[0]);
      return 2;
    }

    const char *option_name = argv[arg_index];
    const char *option_value = argv[++arg_index];
    if (strcmp(option_name, "--config") == 0) {
      config_filename = option_value;
    } else if (strcmp(option_name, "--logger-config") == 0) {
      logger_filename = option_value;
    } else if (strcmp(option_name, "--csv") == 0) {
      csv_filename = option_value;
    } else {
      print_usage(argv[0]);
      return 2;
    }
  }

  // 1. INITIALISATION DU LOGGER
  if (!logger_init(logger_filename)) {
    log_warn("Configuration du logger indisponible; poursuite avec le niveau "
             "INFO.");
  }

  // 2. Appel de l'orchestrateur principal
  ApplicationFiles files = {config_filename, csv_filename, check_only};
  return run_application(&files);
}

static void handle_read_error(const char *config_filename, int read_status) {
  switch (read_status) {
  case READ_ERROR_NULL_CONFIG:
    log_error("Configuration NULL détecté lors de la lecture.");
    break;
  case READ_ERROR_IO_FILE_NOT_FOUND:
    log_error("Impossible de trouver '%s'. Vérifiez le chemin.",
              config_filename);
    break;
  case READ_ERROR_IO_READ:
    log_error("Erreur d’E/S pendant la lecture de '%s'.", config_filename);
    break;
  case READ_ERROR_DATA_CONVERSION:
    log_error("Erreur de conversion des données dans '%s'. Vérifiez le format "
              "des champs.",
              config_filename);
    break;
  case READ_ERROR_DATA_MISSING:
    log_error("Le fichier a été lu, mais aucune clé de configuration valide "
              "n'a été trouvée.");
    break;
  default:
    log_error("Erreur inconnue lors de la lecture des données (Code: %d).",
              read_status);
    break;
  }
}

// ==========================================================================
// ORCHESTRATEUR PRINCIPAL
// ==========================================================================

static int run_application(const ApplicationFiles *files) {

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
  log_info("--- TENTATIVE DE LECTURE DES DONNEES DANS %s ---",
         files->config_filename);
  read_status =
      read_raw_data(files->config_filename, raw); // <-- Utilisation de config_reader

  if (read_status != READ_SUCCESS) {
    handle_read_error(files->config_filename, read_status);
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
    handle_read_error(files->config_filename, read_status);
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

  if (files->check_only) {
    log_info("Configuration valide. Aucune simulation exécutée.");
    return 0;
  }

  // --- 5. Affichage de la préparation (reste inchangé) ---
  log_info("========================================================");
  log_info("--- SIMULATION PRÊTE À DÉMARRER ---");
  log_info("Période: %02d/%02d/%d au %02d/%02d/%d | Latitude: %.2f",
           final.jour_debut, final.mois_debut, final.annee_debut,
           final.jour_fin, final.mois_fin, final.annee_fin, final.latitude);
  log_info("Seuil d'altitude solaire: %.3f°",
           final.altitude_solaire_seuil_degres);

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
  FILE *csv_output = NULL;
  if (files->csv_filename) {
    csv_output = fopen(files->csv_filename, "w");
    if (!csv_output) {
      log_error("Impossible de créer le fichier CSV '%s'.",
                files->csv_filename);
      return 1;
    }
  }

  int simulation_succeeded = run_simulation(&final, csv_output);
  if (csv_output && fclose(csv_output) != 0) {
    log_error("Impossible de finaliser le fichier CSV '%s'.",
              files->csv_filename);
    simulation_succeeded = 0;
  }

  if (!simulation_succeeded) {
    return 1;
  }

  log_info("========================================================");
  log_info("SIMULATION TERMINÉE AVEC SUCCÈS.");
  log_info("========================================================");

  return 0; // Succès final
}
