#include "config_reader.h"
#include "config_validator.h"
#include "date.h"
#include "solar.h"
#include "test_assertions.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

static RawConfig setup_valid_raw_config(void) {
  return (RawConfig){.raw_annee_debut_str = (char *)"2026",
                     .raw_annee_fin_str = (char *)"2026",
                     .raw_latitude_str =
                         (char *)"47 deg 17 min 48 sec Nord",
                     .raw_jour_debut_str = (char *)"21",
                     .raw_mois_debut_str = (char *)"8",
                     .raw_jour_fin_str = (char *)"21",
                     .raw_mois_fin_str = (char *)"8",
                     .raw_mode_solaire_str = (char *)"2"};
}

/* Vérifie que les entiers hors de la plage de conversion sont rejetés. */
static TestResult test_integer_outside_strtol_range_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_status = READ_ERROR_DATA_CONVERSION;
  raw.raw_annee_debut_str =
      (char *)"999999999999999999999999999999999999";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "integer overflow is rejected during conversion");
}

/* Cas de test */

/* Vérifie la conversion, la validation complète et le choix du modèle pour une entrée valide. */
static TestResult test_valid_configuration_is_parsed_and_validated(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_parse_status = READ_SUCCESS;
  const int expected_config_valid = 1;
  const DeclinationFunc expected_declination_function =
      calculate_spencer_declination;

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  TestResult passed = assert_condition(parse_status == expected_parse_status,
                                "valid raw configuration is converted");
  if (parse_status != expected_parse_status) {
    return passed;
  }
  if (assert_condition(is_config_fully_valid(&final) == expected_config_valid,
                       "valid final configuration is accepted") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  if (assert_condition(final.declination_func == expected_declination_function,
                       "mode 2 selects the Spencer calculation") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  if (assert_condition(final.altitude_solaire_seuil_degres == -0.833,
                       "missing solar threshold defaults to -0.833 degrees") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  return passed;
}

/* Vérifie la conversion d'un seuil d'altitude solaire explicitement configuré. */
static TestResult test_solar_altitude_threshold_is_parsed(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_altitude_solaire_seuil_str = (char *)"-0.833";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(
      parse_status == READ_SUCCESS && is_config_fully_valid(&final) &&
          final.altitude_solaire_seuil_degres == -0.833,
      "solar altitude threshold is parsed and validated");
}

/* Vérifie que les seuils d'altitude solaire mal formés sont rejetés. */
static TestResult test_invalid_solar_altitude_threshold_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_altitude_solaire_seuil_str = (char *)"not-a-number";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_ERROR_DATA_CONVERSION,
                          "invalid solar altitude threshold is rejected");
}

/* Vérifie que les seuils hors de la plage angulaire admise sont rejetés. */
static TestResult test_out_of_range_solar_altitude_threshold_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_altitude_solaire_seuil_str = (char *)"90.1";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              !is_config_fully_valid(&final),
                          "out-of-range solar altitude threshold is rejected");
}

/* Vérifie qu'une période valide peut traverser la fin de l'année. */
static TestResult test_cross_year_configuration_is_valid(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_parse_status = READ_SUCCESS;
  raw.raw_jour_debut_str = (char *)"31";
  raw.raw_mois_debut_str = (char *)"12";
  raw.raw_annee_debut_str = (char *)"2025";
  raw.raw_jour_fin_str = (char *)"2";
  raw.raw_mois_fin_str = (char *)"1";
  raw.raw_annee_fin_str = (char *)"2026";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  TestResult passed = assert_condition(parse_status == expected_parse_status,
                                "cross-year configuration is parsed");
  if (parse_status != expected_parse_status) {
    return passed;
  }
  return assert_condition(is_config_fully_valid(&final),
                          "cross-year configuration is accepted");
}

/* Vérifie que le mode 1 sélectionne la fonction de déclinaison sinusoïdale. */
static TestResult test_sinusoidal_mode_selects_sinusoidal_model(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const DeclinationFunc expected_function = calculate_sinusoidal_declination;
  raw.raw_mode_solaire_str = (char *)"1";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              final.declination_func == expected_function,
                          "mode 1 selects the sinusoidal model");
}

/* Vérifie que le mode 3 sélectionne la fonction de déclinaison de Meeus. */
static TestResult test_meeus_mode_selects_meeus_model(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const DeclinationFunc expected_function = calculate_meeus_declination;
  raw.raw_mode_solaire_str = (char *)"3";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              final.declination_func == expected_function,
                          "mode 3 selects the Meeus model");
}

/* Vérifie que l'analyse rejette une configuration brute NULL. */
static TestResult test_null_raw_configuration_is_rejected(void) {
  /* Given */
  FinalConfig final;
  const int expected_status = READ_ERROR_NULL_CONFIG;

  /* When */
  int parse_status = parse_and_validate_config(NULL, &final);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "null raw configuration is rejected");
}

/* Vérifie que l'analyse rejette une configuration de sortie NULL. */
static TestResult test_null_final_configuration_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  const int expected_status = READ_ERROR_NULL_CONFIG;

  /* When */
  int parse_status = parse_and_validate_config(&raw, NULL);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "null final configuration is rejected");
}

/* Vérifie que la validation complète rejette une configuration finale NULL. */
static TestResult test_null_final_configuration_fails_full_validation(void) {
  /* Given */
  const FinalConfig *final = NULL;

  /* When */
  bool valid = is_config_fully_valid(final);

  /* Then */
  return assert_condition(!valid,
                          "null final configuration fails full validation");
}

/* Vérifie que les champs entiers suivis de texte non numérique sont rejetés. */
static TestResult test_invalid_integer_suffix_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_status = READ_ERROR_DATA_CONVERSION;
  raw.raw_jour_fin_str = (char *)"21jours";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "integer values with trailing text are rejected");
}

/* Vérifie qu'une date inexistante dans le calendrier est rejetée. */
static TestResult test_invalid_end_date_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_jour_fin_str = (char *)"31";
  raw.raw_mois_fin_str = (char *)"4";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              !is_config_fully_valid(&final),
                          "a nonexistent end date is rejected");
}

/* Vérifie qu'une année de début inférieure au minimum pris en charge est rejetée. */
static TestResult test_start_year_below_supported_range_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_annee_debut_str = (char *)"1899";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              !is_config_fully_valid(&final),
                          "a start year below the supported range is rejected");
}

/* Vérifie qu'une année de fin supérieure au maximum pris en charge est rejetée. */
static TestResult test_end_year_above_supported_range_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_annee_fin_str = (char *)"3001";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              !is_config_fully_valid(&final),
                          "an end year above the supported range is rejected");
}

/* Vérifie que les latitudes au-delà des pôles sont rejetées. */
static TestResult test_latitudes_outside_supported_range_are_rejected(void) {
  /* Given */
  RawConfig north_raw = setup_valid_raw_config();
  RawConfig south_raw = setup_valid_raw_config();
  FinalConfig north_final;
  FinalConfig south_final;
  north_raw.raw_latitude_str = (char *)"90 deg 1 min 0 sec Nord";
  south_raw.raw_latitude_str = (char *)"90 deg 1 min 0 sec Sud";

  /* When */
  int north_parse_status = parse_and_validate_config(&north_raw, &north_final);
  int south_parse_status = parse_and_validate_config(&south_raw, &south_final);

  /* Then */
  return assert_condition(
      north_parse_status == READ_SUCCESS &&
          !is_config_fully_valid(&north_final) &&
          south_parse_status == READ_SUCCESS &&
          !is_config_fully_valid(&south_final),
      "latitudes above 90 degrees in either hemisphere are rejected");
}

/* Vérifie qu'une date de début postérieure à la date de fin échoue à la validation. */
static TestResult test_reversed_dates_fail_final_validation(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  raw.raw_jour_debut_str = (char *)"22";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == READ_SUCCESS &&
                              !is_config_fully_valid(&final),
                          "a reversed period fails final configuration validation");
}

/* Vérifie que l'ordre des dates rejette les périodes contenant une date invalide. */
static TestResult test_time_range_rejects_invalid_dates(void) {
  /* Given */
  const DateRange range = {{31, 4, 2026}, {1, 5, 2026}};
  const int expected_valid = 0;

  /* When */
  int valid = is_date_range_ordered(range);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "a time range with an invalid date is rejected");
}

/* Vérifie qu'une période correctement ordonnée est acceptée. */
static TestResult test_chronological_date_range_is_accepted(void) {
  /* Given */
  const DateRange range = {{21, 8, 2026}, {22, 8, 2026}};
  const int expected_valid = 1;

  /* When */
  int valid = is_date_range_ordered(range);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "chronological date range is accepted");
}

/* Vérifie qu'une période inversée dans une même année est rejetée. */
static TestResult test_inverted_date_range_is_rejected(void) {
  /* Given */
  const DateRange range = {{22, 8, 2026}, {21, 8, 2026}};
  const int expected_valid = 0;

  /* When */
  int valid = is_date_range_ordered(range);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "inverted date range is rejected");
}

/* Vérifie que l'ordre chronologique est respecté lors du passage à une nouvelle année. */
static TestResult test_date_range_crossing_year_is_accepted(void) {
  /* Given */
  const DateRange range = {{31, 12, 2025}, {2, 1, 2026}};
  const int expected_valid = 1;

  /* When */
  int valid = is_date_range_ordered(range);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "date range crossing a year is accepted");
}

/* Vérifie qu'une période se terminant une année antérieure est rejetée. */
static TestResult test_date_range_with_end_before_start_is_rejected(void) {
  /* Given */
  const DateRange range = {{1, 1, 2026}, {31, 12, 2025}};
  const int expected_valid = 0;

  /* When */
  int valid = is_date_range_ordered(range);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "date range ending before its start is rejected");
}

/* Vérifie que les numéros de modèle solaire non pris en charge sont rejetés. */
static TestResult test_unknown_solar_mode_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_status = READ_ERROR_DATA_CONVERSION;
  raw.raw_mode_solaire_str = (char *)"99";

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "unknown solar mode is rejected");
}

/* Vérifie qu'un champ obligatoire manquant est rejeté. */
static TestResult test_missing_configuration_field_is_rejected(void) {
  /* Given */
  RawConfig raw = setup_valid_raw_config();
  FinalConfig final;
  const int expected_status = READ_ERROR_DATA_MISSING;
  raw.raw_mode_solaire_str = NULL;

  /* When */
  int parse_status = parse_and_validate_config(&raw, &final);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "missing configuration field is rejected");
}

static const TestCase test_cases[] = {
    {"test_valid_configuration_is_parsed_and_validated",
     test_valid_configuration_is_parsed_and_validated},
    {"test_cross_year_configuration_is_valid",
     test_cross_year_configuration_is_valid},
    {"test_solar_altitude_threshold_is_parsed",
     test_solar_altitude_threshold_is_parsed},
    {"test_invalid_solar_altitude_threshold_is_rejected",
     test_invalid_solar_altitude_threshold_is_rejected},
    {"test_out_of_range_solar_altitude_threshold_is_rejected",
     test_out_of_range_solar_altitude_threshold_is_rejected},
    {"test_sinusoidal_mode_selects_sinusoidal_model",
     test_sinusoidal_mode_selects_sinusoidal_model},
    {"test_meeus_mode_selects_meeus_model",
     test_meeus_mode_selects_meeus_model},
    {"test_null_raw_configuration_is_rejected",
     test_null_raw_configuration_is_rejected},
    {"test_null_final_configuration_is_rejected",
     test_null_final_configuration_is_rejected},
    {"test_null_final_configuration_fails_full_validation",
     test_null_final_configuration_fails_full_validation},
    {"test_invalid_integer_suffix_is_rejected",
     test_invalid_integer_suffix_is_rejected},
    {"test_invalid_end_date_is_rejected", test_invalid_end_date_is_rejected},
    {"test_start_year_below_supported_range_is_rejected",
     test_start_year_below_supported_range_is_rejected},
    {"test_end_year_above_supported_range_is_rejected",
     test_end_year_above_supported_range_is_rejected},
    {"test_latitudes_outside_supported_range_are_rejected",
     test_latitudes_outside_supported_range_are_rejected},
    {"test_reversed_dates_fail_final_validation",
     test_reversed_dates_fail_final_validation},
    {"test_time_range_rejects_invalid_dates",
     test_time_range_rejects_invalid_dates},
    {"test_chronological_date_range_is_accepted",
     test_chronological_date_range_is_accepted},
    {"test_inverted_date_range_is_rejected",
     test_inverted_date_range_is_rejected},
    {"test_date_range_crossing_year_is_accepted",
     test_date_range_crossing_year_is_accepted},
    {"test_date_range_with_end_before_start_is_rejected",
     test_date_range_with_end_before_start_is_rejected},
    {"test_unknown_solar_mode_is_rejected", test_unknown_solar_mode_is_rejected},
    {"test_integer_outside_strtol_range_is_rejected",
     test_integer_outside_strtol_range_is_rejected},
    {"test_missing_configuration_field_is_rejected",
     test_missing_configuration_field_is_rejected},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("config_validator", test_cases,
                       sizeof(test_cases) / sizeof(test_cases[0]));
}