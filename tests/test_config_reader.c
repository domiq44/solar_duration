#include <string.h>

#include "config_reader.h"
#include "test_assertions.h"
#include "test_file_utils.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

static const char *valid_config_contents =
    "# config comment\n"
    "ANNEE_DEBUT = 2026 // inline comment\n"
    "ANNEE_FIN = 2026\n"
    "latitude = 47 deg 17 min 48 sec Nord\n"
    "jour_debut = 21\n"
    "mois_debut = 8\n"
    "jour_fin = 22\n"
    "mois_fin = 8\n"
    "mode_solaire = 2\n"
    "option_inconnue = ignoree";

static const RawConfig expected_raw_config = {
  (char *)"2026", (char *)"2026", (char *)"47 deg 17 min 48 sec Nord",
  (char *)"21", (char *)"8", (char *)"22", (char *)"8", (char *)"2",
  NULL};

/* Cas de test */

/* Vérifie qu'une nouvelle configuration brute initialise tous ses champs à NULL. */
static TestResult test_create_raw_config_initializes_all_fields(void) {
  /* Given */
  RawConfig *raw = NULL;
  const int expected_fields_are_null = 1;

  /* When */
  raw = create_raw_config();

  /* Then */
  TestResult passed =
      assert_condition(raw != NULL, "raw configuration is allocated");
  if (raw) {
    int fields_are_null =
        raw->raw_annee_debut_str == NULL && raw->raw_annee_fin_str == NULL &&
        raw->raw_latitude_str == NULL &&
        raw->raw_jour_debut_str == NULL && raw->raw_mois_debut_str == NULL &&
        raw->raw_jour_fin_str == NULL && raw->raw_mois_fin_str == NULL &&
        raw->raw_mode_solaire_str == NULL &&
        raw->raw_altitude_solaire_seuil_str == NULL;
    if (assert_condition(fields_are_null == expected_fields_are_null,
                         "all raw fields start as null") == TEST_FAILED) {
      passed = TEST_FAILED;
    }
    free_raw_config(raw);
  }
  return passed;
}

/* Vérifie que la dernière occurrence d'une clé dupliquée est conservée. */
static TestResult
test_read_raw_data_accepts_duplicate_keys_with_last_value(void) {
  /* Given */
  char path[64];
  const char *contents =
      "annee_debut = 2026\n"
      "annee_fin = 2026\n"
      "latitude = 47 deg 17 min 48 sec Nord\n"
      "jour_debut = 21\n"
      "mois_debut = 8\n"
      "jour_fin = 22\n"
      "mois_fin = 8\n"
      "mode_solaire = 2\n"
      "annee_debut = 2027\n";
  const int expected_status = READ_SUCCESS;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(path, sizeof(path), contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration file is created");
  }

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(read_status == expected_status,
                                "duplicate configuration keys are accepted");
  if (read_status == expected_status) {
    if (assert_condition(strcmp(raw->raw_annee_debut_str, "2027") == 0,
                         "the last duplicate value is retained") ==
        TEST_FAILED) {
      passed = TEST_FAILED;
    }
  }
  free_raw_config(raw);
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que les commentaires et l'absence de saut de ligne final sont acceptés. */
static TestResult
test_read_raw_data_accepts_comments_and_missing_final_newline(void) {
  /* Given */
  char path[64];
  const int expected_status = READ_SUCCESS;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(path, sizeof(path), valid_config_contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration file is created");
  }

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(read_status == expected_status,
                                "complete configuration is read successfully");
  if (read_status == expected_status) {
    int values_match =
         strcmp(raw->raw_annee_debut_str,
           expected_raw_config.raw_annee_debut_str) == 0 &&
         strcmp(raw->raw_annee_fin_str,
           expected_raw_config.raw_annee_fin_str) == 0 &&
        strcmp(raw->raw_latitude_str, expected_raw_config.raw_latitude_str) ==
            0 &&
        strcmp(raw->raw_jour_debut_str,
               expected_raw_config.raw_jour_debut_str) == 0 &&
        strcmp(raw->raw_mois_debut_str,
               expected_raw_config.raw_mois_debut_str) == 0 &&
        strcmp(raw->raw_jour_fin_str, expected_raw_config.raw_jour_fin_str) ==
            0 &&
        strcmp(raw->raw_mois_fin_str, expected_raw_config.raw_mois_fin_str) ==
            0 &&
        strcmp(raw->raw_mode_solaire_str,
               expected_raw_config.raw_mode_solaire_str) == 0;
    if (assert_condition(values_match,
                         "all required values are trimmed and stored") ==
        TEST_FAILED) {
      passed = TEST_FAILED;
    }
  }
  free_raw_config(raw);
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que le seuil d'altitude solaire est une clé facultative. */
static TestResult test_read_raw_data_accepts_optional_solar_altitude(void) {
  /* Given */
  char path[64];
  const char *contents =
      "annee_debut = 2026\n"
      "annee_fin = 2026\n"
      "latitude = 47 deg 17 min 48 sec Nord\n"
      "jour_debut = 21\n"
      "mois_debut = 8\n"
      "jour_fin = 22\n"
      "mois_fin = 8\n"
      "mode_solaire = 2\n"
      "altitude_solaire_seuil = -0.833\n";
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(path, sizeof(path), contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration file is created");
  }

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(
      read_status == READ_SUCCESS &&
          strcmp(raw->raw_altitude_solaire_seuil_str, "-0.833") == 0,
      "optional solar altitude threshold is read");
  free_raw_config(raw);
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que les configurations auxquelles il manque des clés obligatoires sont rejetées. */
static TestResult test_read_raw_data_rejects_incomplete_config(void) {
  /* Given */
  char path[64];
  const char *contents = "annee_debut = 2026\n";
  const int expected_status = READ_ERROR_DATA_MISSING;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(path, sizeof(path), contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration file is created");
  }

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(read_status == expected_status,
                                "configuration missing required keys is rejected");
  free_raw_config(raw);
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que les lignes de configuration trop longues sont rejetées et réinitialisées. */
static TestResult test_read_raw_data_rejects_overlong_line(void) {
  /* Given */
  char path[64];
  char contents[320] = "annee_debut = 2026\n#";
  const int expected_status = READ_ERROR_DATA_CONVERSION;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  memset(contents + strlen(contents), 'x', 280);
  contents[strlen("annee_debut = 2026\n#") + 280] = '\n';
  contents[strlen("annee_debut = 2026\n#") + 281] = '\0';
  if (!create_temporary_file(path, sizeof(path), contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration file is created");
  }

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(
      read_status == expected_status,
      "configuration with an overlong line is rejected");
  if (assert_condition(raw->raw_annee_debut_str == NULL,
                       "partial configuration is cleared after rejection") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  free_raw_config(raw);
  remove_temporary_file(path);
  return passed;
}

/* Vérifie qu'une nouvelle lecture ne conserve pas les valeurs du fichier précédent. */
static TestResult test_read_raw_data_does_not_reuse_previous_values(void) {
  /* Given */
  char complete_path[64];
  char incomplete_path[64];
  const char *incomplete_contents = "annee_debut = 2027\n";
  const int expected_status = READ_ERROR_DATA_MISSING;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(complete_path, sizeof(complete_path),
                             valid_config_contents)) {
    free_raw_config(raw);
    return assert_condition(0, "temporary configuration files are created");
  }
  if (!create_temporary_file(incomplete_path, sizeof(incomplete_path),
                             incomplete_contents)) {
    free_raw_config(raw);
    remove_temporary_file(complete_path);
    return assert_condition(0, "temporary configuration files are created");
  }
  int initial_status = read_raw_data(complete_path, raw);
  if (initial_status != READ_SUCCESS) {
    free_raw_config(raw);
    remove_temporary_file(complete_path);
    remove_temporary_file(incomplete_path);
    return assert_condition(0, "complete configuration is read first");
  }

  /* When */
  int read_status = read_raw_data(incomplete_path, raw);

  /* Then */
  TestResult passed = assert_condition(
      read_status == expected_status,
      "incomplete configuration is rejected after a complete read");
  free_raw_config(raw);
  remove_temporary_file(complete_path);
  remove_temporary_file(incomplete_path);
  return passed;
}

/* Vérifie qu'un fichier de configuration absent renvoie une erreur d'E/S. */
static TestResult test_read_raw_data_rejects_missing_file(void) {
  /* Given */
  char path[64];
  const int expected_status = READ_ERROR_IO_FILE_NOT_FOUND;
  RawConfig *raw = create_raw_config();
  if (!raw) {
    return assert_condition(0, "raw configuration is allocated");
  }
  if (!create_temporary_file(path, sizeof(path), "")) {
    free_raw_config(raw);
    return assert_condition(0, "temporary path is created");
  }
  remove_temporary_file(path);

  /* When */
  int read_status = read_raw_data(path, raw);

  /* Then */
  TestResult passed = assert_condition(read_status == expected_status,
                                "missing configuration file is rejected");
  free_raw_config(raw);
  return passed;
}

/* Vérifie que la lecture dans une configuration NULL est rejetée. */
static TestResult test_read_raw_data_rejects_null_config(void) {
  /* Given */
  const int expected_status = READ_ERROR_NULL_CONFIG;

  /* When */
  int read_status = read_raw_data("unused-path", NULL);

  /* Then */
  return assert_condition(read_status == expected_status,
                          "null raw configuration is rejected");
}

static const TestCase test_cases[] = {
    {"test_create_raw_config_initializes_all_fields",
     test_create_raw_config_initializes_all_fields},
    {"test_read_raw_data_accepts_comments_and_missing_final_newline",
     test_read_raw_data_accepts_comments_and_missing_final_newline},
    {"test_read_raw_data_accepts_optional_solar_altitude",
     test_read_raw_data_accepts_optional_solar_altitude},
    {"test_read_raw_data_accepts_duplicate_keys_with_last_value",
     test_read_raw_data_accepts_duplicate_keys_with_last_value},
    {"test_read_raw_data_rejects_incomplete_config",
     test_read_raw_data_rejects_incomplete_config},
    {"test_read_raw_data_rejects_overlong_line",
     test_read_raw_data_rejects_overlong_line},
    {"test_read_raw_data_does_not_reuse_previous_values",
     test_read_raw_data_does_not_reuse_previous_values},
    {"test_read_raw_data_rejects_missing_file",
     test_read_raw_data_rejects_missing_file},
    {"test_read_raw_data_rejects_null_config",
     test_read_raw_data_rejects_null_config},
};

int main(void) {
  return run_test_suite("config_reader", test_cases,
                       sizeof(test_cases) / sizeof(test_cases[0]));
}