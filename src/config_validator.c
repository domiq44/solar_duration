// --- Fichier : config_validator.c ---

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
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

static bool parse_integer_value(const char *value, int *result) {
  errno = 0;
  char *endptr;
  long parsed_value = strtol(value, &endptr, 10);
  if (errno == ERANGE || endptr == value || *endptr != '\0' ||
      parsed_value < INT_MIN || parsed_value > INT_MAX) {
    return false;
  }

  *result = (int)parsed_value;
  return true;
}

static bool parse_double_value(const char *value, double *result) {
  errno = 0;
  char *endptr;
  double parsed_value = strtod(value, &endptr);
  if (errno == ERANGE || endptr == value || *endptr != '\0' ||
      !isfinite(parsed_value)) {
    return false;
  }

  *result = parsed_value;
  return true;
}

static DeclinationFunc declination_function_for_mode(int mode) {
  switch (mode) {
  case MODE_SINUSOIDAL:
    return calculate_sinusoidal_declination;
  case MODE_SPENCER:
    return calculate_spencer_declination;
  case MODE_MEEUS:
    return calculate_meeus_declination;
  default:
    return NULL;
  }
}

// Initialise le pointeur de fonction dans FinalConfig.
static void initialize_config_functions(FinalConfig *final) {
  final->declination_func =
      declination_function_for_mode(final->mode_declinaison);
}

// ==========================================================================
// CONVERSION ET VALIDATION (RawConfig -> FinalConfig)
// ==========================================================================

int parse_and_validate_config(const RawConfig *raw, FinalConfig *final) {
  if (!raw || !final)
    return READ_ERROR_NULL_CONFIG;

  // --- Vérification de la présence des champs avant conversion ---
  if (!raw->raw_annee_debut_str || !raw->raw_annee_fin_str ||
      !raw->raw_latitude_str || !raw->raw_jour_debut_str ||
      !raw->raw_mois_debut_str || !raw->raw_jour_fin_str ||
      !raw->raw_mois_fin_str || !raw->raw_mode_solaire_str) {
    log_error(
        "Configuration incomplète: Un ou plusieurs champs sont manquants.");
    return READ_ERROR_DATA_MISSING;
  }
  // -----------------------------------------------------------------

  // 1. Conversion des années
  log_debug("DEBUG_PARSE: Tentative de conversion de l'année de début. Chaîne "
            "brute: [%s]",
            raw->raw_annee_debut_str);
  if (!parse_integer_value(raw->raw_annee_debut_str, &final->annee_debut)) {
    log_error("ERREUR DE CONVERSION: 'annee_debut' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }
  if (!parse_integer_value(raw->raw_annee_fin_str, &final->annee_fin)) {
    log_error("ERREUR DE CONVERSION: 'annee_fin' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // 2. Conversion de la Latitude
  // (La latitude est gérée par parse_latitude_string, ce qui est déjà bien)
  double lat_val;
  int err_geo = parse_latitude_string(raw->raw_latitude_str, &lat_val);
  if (err_geo != ERR_OK) {
    log_error("ERREUR DE CONVERSION: Impossible de parser la latitude '%s'.",
              raw->raw_latitude_str);
    return READ_ERROR_DATA_CONVERSION;
  }
  final->latitude = lat_val;

  final->altitude_solaire_seuil_degres =
      DEFAULT_SOLAR_ALTITUDE_THRESHOLD_DEGREES;
  if (raw->raw_altitude_solaire_seuil_str &&
      !parse_double_value(raw->raw_altitude_solaire_seuil_str,
                          &final->altitude_solaire_seuil_degres)) {
    log_error("ERREUR DE CONVERSION: 'altitude_solaire_seuil' invalide.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // 3. Conversion des Dates (Jour/Mois Début & Fin)

  // Jour de début
  log_debug("DEBUG_PARSE: Tentative de convertir le jour de début. Chaîne "
            "brute: [%s]",
            raw->raw_jour_debut_str);
  if (!parse_integer_value(raw->raw_jour_debut_str, &final->jour_debut)) {
    log_error("ERREUR DE CONVERSION: 'jour_debut' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // Mois de début
  log_debug("DEBUG_PARSE: Tentative de convertir le mois de début. Chaîne "
            "brute: [%s]",
            raw->raw_mois_debut_str);
  if (!parse_integer_value(raw->raw_mois_debut_str, &final->mois_debut)) {
    log_error("ERREUR DE CONVERSION: 'mois_debut' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // Jour de fin
  log_debug(
      "DEBUG_PARSE: Tentative de convertir le jour de fin. Chaîne brute: [%s]",
      raw->raw_jour_fin_str);
  if (!parse_integer_value(raw->raw_jour_fin_str, &final->jour_fin)) {
    log_error("ERREUR DE CONVERSION: 'jour_fin' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // Mois de fin
  log_debug(
      "DEBUG_PARSE: Tentative de convertir le mois de fin. Chaîne brute: [%s]",
      raw->raw_mois_fin_str);
  if (!parse_integer_value(raw->raw_mois_fin_str, &final->mois_fin)) {
    log_error("ERREUR DE CONVERSION: 'mois_fin' invalide dans la ligne.");
    return READ_ERROR_DATA_CONVERSION;
  }

  // 4. Conversion du Mode Solaire
  log_debug(
      "DEBUG_PARSE: Tentative de convertir le mode solaire. Chaîne brute: [%s]",
      raw->raw_mode_solaire_str);
  if (!parse_integer_value(raw->raw_mode_solaire_str,
                           &final->mode_declinaison)) {
    log_error("ERREUR DE CONVERSION: 'mode_solaire' invalide.");
    return READ_ERROR_DATA_CONVERSION;
  }
  if (final->mode_declinaison < MODE_SINUSOIDAL ||
      final->mode_declinaison > MODE_MEEUS) {
    log_error("ERREUR FATALE: Mode solaire '%ld' n'est pas reconnu. Doit être "
              "entre %d et %d.",
              (long) final->mode_declinaison, MODE_SINUSOIDAL, MODE_MEEUS);
    return READ_ERROR_DATA_CONVERSION;
  }

  // 5. Initialisation des fonctions de calcul
  initialize_config_functions(final);

  return READ_SUCCESS;
}

// ==========================================================================
// VALIDATION FINALE (Logique métier)
// ==========================================================================

bool is_config_fully_valid(const FinalConfig *final) {
  if (!final) {
    log_error("ERREUR DE DONNÉES: Configuration finale NULL.");
    return false;
  }

  CalendarDate start_date = {final->jour_debut, final->mois_debut,
                             final->annee_debut};
  CalendarDate end_date = {final->jour_fin, final->mois_fin,
                           final->annee_fin};

  // 1. Validation des années
  if (final->annee_debut < MIN_YEAR || final->annee_debut > MAX_YEAR ||
      final->annee_fin < MIN_YEAR || final->annee_fin > MAX_YEAR) {
    log_error("ERREUR DE DONNEES: Année de début ou de fin hors plage valide.");
    return false;
  }
  // 2. Validation Latitude
  if (!isfinite(final->latitude) || final->latitude < MIN_LATITUDE ||
      final->latitude > MAX_LATITUDE) {
    log_error("ERREUR DE DONNEES: Latitude hors plage valide.");
    return false;
  }

  if (!isfinite(final->altitude_solaire_seuil_degres) ||
      final->altitude_solaire_seuil_degres < MIN_SOLAR_ALTITUDE_DEGREES ||
      final->altitude_solaire_seuil_degres > MAX_SOLAR_ALTITUDE_DEGREES) {
    log_error("ERREUR DE DONNEES: Seuil d'altitude solaire hors plage valide.");
    return false;
  }

  // 3. Validation structurelle des dates (utilise les fonctions de date.c)
  if (!is_date_valid(start_date)) {
    log_error("ERREUR DE DONNÉES: Date de début (%02d/%02d/%d) est "
              "structurellement invalide.",
              final->jour_debut, final->mois_debut, final->annee_debut);
    return false;
  }
  if (!is_date_valid(end_date)) {
    log_error("ERREUR DE DONNÉES: Date de fin (%02d/%02d/%d) est "
              "structurellement invalide.",
              final->jour_fin, final->mois_fin, final->annee_fin);
    return false;
  }

  // 4. Validation temporelle (Début doit être avant ou égal à Fin)
  if (!is_date_range_ordered((DateRange){start_date, end_date})) {
    log_error("ERREUR DE DONNÉES: La date de début (%02d/%02d/%d) est "
              "postérieure à la date de fin (%02d/%02d/%d).",
              final->jour_debut, final->mois_debut, final->annee_debut,
              final->jour_fin, final->mois_fin, final->annee_fin);
    return false;
  }

  // 5. Validation du mode solaire
  if (final->mode_declinaison < MODE_SINUSOIDAL ||
      final->mode_declinaison > MODE_MEEUS) {
    log_error("ERREUR DE DONNÉES: Mode de déclinaison hors plage valide.");
    return false;
  }

  // 6. Validation de la correspondance mode / fonction de déclinaison
  if (final->declination_func !=
      declination_function_for_mode(final->mode_declinaison)) {
    log_error(
        "ERREUR INTERNE: Pointeur de fonction déclinaison incohérent. "
        "Mode de déclinaison: %d",
        final->mode_declinaison);
    return false;
  }

  return true; // Succès
}
