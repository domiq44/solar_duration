// --- File: config_validator.c ---

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "config_reader.h"
#include "config_validator.h"
#include "date.h"
#include "geo.h"
#include "logger.h"
#include "string_utils.h"

// ==========================================================================
// UTILITAIRES INTERNES
// ==========================================================================

// Initialise les pointeurs de fonctions dans FinalConfig.
static void initialize_config_functions(FinalConfig *final) {
  final->declination_func = NULL;
  switch (final->mode_declinaison) {
  case MODE_SINUSOIDAL:
    final->declination_func = calculate_sinusoidal_declination;
    break;
  case MODE_SPENCER:
    final->declination_func = calculate_spencer_declination;
    break;
  case MODE_MEEUS:
    final->declination_func = calculate_meeus_declination;
    break;
  default:
    // L'erreur sera capturée par is_config_fully_valid
    break;
  }
}

/**
 * @brief Vérifie que deux dates sont ordonnées temporellement (d1 <= d2).
 * @param jour_debut, mois_debut, annee_debut : Première date
 * @param jour_fin, mois_fin, annee_fin : Deuxième date
 * @return bool true si d1 <= d2, false sinon
 */
static bool are_dates_ordered(int jour_debut, int mois_debut, int annee_debut,
                              int jour_fin, int mois_fin, int annee_fin) {
  int ordinal_debut = date_to_ordinal(jour_debut, mois_debut, annee_debut);
  int ordinal_fin = date_to_ordinal(jour_fin, mois_fin, annee_fin);
  return ordinal_debut <= ordinal_fin;
}

// ==========================================================================
// CONVERSION ET VALIDATION (RawConfig -> FinalConfig)
// ==========================================================================

int parse_and_validate_config(const RawConfig *raw, FinalConfig *final) {
  if (!raw || !final)
    return READ_ERROR_NULL_CONFIG;

  // --- Vérification de la présence des champs avant conversion ---
  if (!raw->raw_annee_str || !raw->raw_latitude_str ||
      !raw->raw_jour_debut_str || !raw->raw_mois_debut_str ||
      !raw->raw_jour_fin_str || !raw->raw_mois_fin_str ||
      !raw->raw_mode_solaire_str) {
    log_error(
        "Configuration incomplète: Un ou plusieurs champs sont manquants.");
    return READ_ERROR_DATA_MISSING;
  }
  // -----------------------------------------------------------------

  // 1. Conversion de l'Année
  char *endptr;
  long temp_year = strtol(raw->raw_annee_str, &endptr, 10);
  if (endptr == raw->raw_annee_str || *endptr != '\0') {
    log_error("ERREUR DE CONVERSION: 'annee' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }
  final->annee = (int)temp_year;

  // 2. Conversion de la Latitude
  double lat_val;
  int err_geo = parse_latitude_string(raw->raw_latitude_str, &lat_val);
  if (err_geo != ERR_OK) {
    log_error("ERREUR DE CONVERSION: Impossible de parser la latitude '%s'.",
              raw->raw_latitude_str);
    return READ_ERROR_DATA_CONVERSION;
  }
  final->latitude = lat_val;

  // 3. Conversion des Dates (Jour/Mois Début & Fin)
  // Jour de début
  long temp_jd = strtol(raw->raw_jour_debut_str, &endptr, 10);
  if (endptr == raw->raw_jour_debut_str || *endptr != '\0')
    return READ_ERROR_DATA_CONVERSION;
  final->jour_debut = (int)temp_jd;

  // Mois de début
  long temp_md = strtol(raw->raw_mois_debut_str, &endptr, 10);
  if (endptr == raw->raw_mois_debut_str || *endptr != '\0')
    return READ_ERROR_DATA_CONVERSION;
  final->mois_debut = (int)temp_md;

  // Jour de fin
  temp_jd = strtol(raw->raw_jour_fin_str, &endptr, 10);
  if (endptr == raw->raw_jour_fin_str || *endptr != '\0')
    return READ_ERROR_DATA_CONVERSION;
  final->jour_fin = (int)temp_jd;

  // Mois de fin
  temp_md = strtol(raw->raw_mois_fin_str, &endptr, 10);
  if (endptr == raw->raw_mois_fin_str || *endptr != '\0')
    return READ_ERROR_DATA_CONVERSION;
  final->mois_fin = (int)temp_md;

  // 4. Conversion du Mode Solaire
  long temp_mode = strtol(raw->raw_mode_solaire_str, &endptr, 10);
  if (endptr == raw->raw_mode_solaire_str || *endptr != '\0') {
    log_error("ERREUR DE CONVERSION: 'mode_solaire' invalide.");
    return READ_ERROR_DATA_CONVERSION;
  }
  if (temp_mode < MODE_SINUSOIDAL || temp_mode > MODE_MEEUS) {
    log_error("ERREUR FATALE: Mode solaire '%ld' n'est pas reconnu. Doit être "
              "entre %d et %d.",
              temp_mode, MODE_SINUSOIDAL, MODE_MEEUS);
    return READ_ERROR_DATA_CONVERSION;
  }
  final->mode_declinaison = (int)temp_mode;

  // 5. Initialisation des fonctions de calcul
  initialize_config_functions(final);

  return READ_SUCCESS;
}

// ==========================================================================
// VALIDATION FINALE (Logique métier)
// ==========================================================================

bool is_config_fully_valid(const FinalConfig *final) {
  // 1. Validation Année
  if (final->annee < MIN_YEAR || final->annee > MAX_YEAR) {
    log_error("ERREUR DE DONNEES: Année hors plage valide.");
    return false;
  }
  // 2. Validation Latitude
  if (final->latitude < MIN_LATITUDE || final->latitude > MAX_LATITUDE) {
    log_error("ERREUR DE DONNEES: Latitude hors plage valide.");
    return false;
  }

  // 3. Validation structurelle des dates (utilise les fonctions de date.c)
  if (!is_date_valid(final->jour_debut, final->mois_debut, final->annee)) {
    log_error("ERREUR DE DONNÉES: Date de début (%02d/%02d/%d) est "
              "structurellement invalide.",
              final->jour_debut, final->mois_debut, final->annee);
    return false;
  }
  if (!is_date_valid(final->jour_fin, final->mois_fin, final->annee)) {
    log_error("ERREUR DE DONNÉES: Date de fin (%02d/%02d/%d) est "
              "structurellement invalide.",
              final->jour_fin, final->mois_fin, final->annee);
    return false;
  }

  // 4. Validation temporelle (Début doit être avant ou égal à Fin)
  if (!are_dates_ordered(final->jour_debut, final->mois_debut, final->annee,
                         final->jour_fin, final->mois_fin, final->annee)) {
    int ordinal_debut =
        date_to_ordinal(final->jour_debut, final->mois_debut, final->annee);
    int ordinal_fin =
        date_to_ordinal(final->jour_fin, final->mois_fin, final->annee);
    log_error("ERREUR DE DONNÉES: La date de début (%d) est postérieure à la "
              "date de fin (%d).",
              ordinal_debut, ordinal_fin);
    return false;
  }

  // 5. Validation du mode solaire
  if (final->mode_declinaison < MODE_SINUSOIDAL ||
      final->mode_declinaison > MODE_MEEUS) {
    log_error("ERREUR DE DONNÉES: Mode de déclinaison hors plage valide.");
    return false;
  }

  // 6. Validation du pointeur fonction déclinaison
  if (final->declination_func == NULL) {
    log_error(
        "ERREUR INTERNE: Pointeur de fonction déclinaison non initialisé. "
        "Mode de déclinaison: %d",
        final->mode_declinaison);
    return false;
  }

  return true; // Succès
}

bool is_time_range_valid(int d_start, int m_start, int d_end, int m_end,
                         int annee) {
  // ... (Implémentation inchangée, utilise date_to_ordinal) ...
  int ordinal_debut = date_to_ordinal(d_start, m_start, annee);
  int ordinal_fin = date_to_ordinal(d_end, m_end, annee);

  if (ordinal_debut == -1 || ordinal_fin == -1) {
    return false;
  }
  return ordinal_debut <= ordinal_fin;
}

// ==========================================================================
// GESTION DES ERREURS
// ==========================================================================

void handle_read_error(int read_status) {
  switch (read_status) {
  case READ_ERROR_NULL_CONFIG:
    log_error("Configuration NULL détecté lors de la lecture.");
    break;
  case READ_ERROR_IO_FILE_NOT_FOUND:
    log_error(
        "Impossible de trouver 'solar_duration.dat'. Vérifiez le chemin.");
    break;
  case READ_ERROR_DATA_CONVERSION:
    log_error("Erreur de conversion des données dans 'solar_duration.dat'. "
              "Vérifiez le format des champs.");
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
