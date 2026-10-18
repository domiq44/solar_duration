#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "simulation.h"
#include "solar.h"
#include "test_assertions.h"
#include "test_runner.h"

typedef struct {
  FILE *stream;
  int saved_stdout;
} StdoutCapture;

/* Fonctions auxiliaires */

static int start_stdout_capture(StdoutCapture *capture) {
  capture->stream = tmpfile();
  if (!capture->stream) {
    return 0;
  }

  capture->saved_stdout = dup(STDOUT_FILENO);
  if (capture->saved_stdout < 0) {
    fclose(capture->stream);
    return 0;
  }

  fflush(stdout);
  if (dup2(fileno(capture->stream), STDOUT_FILENO) < 0) {
    close(capture->saved_stdout);
    fclose(capture->stream);
    return 0;
  }
  return 1;
}

static int finish_stdout_capture(StdoutCapture *capture, char *output,
                                 size_t output_size) {
  fflush(stdout);
  rewind(capture->stream);
  size_t bytes_read = fread(output, 1, output_size - 1, capture->stream);
  int read_succeeded = !ferror(capture->stream);
  output[bytes_read] = '\0';

  int restore_succeeded = dup2(capture->saved_stdout, STDOUT_FILENO) >= 0;
  close(capture->saved_stdout);
  fclose(capture->stream);
  return read_succeeded && restore_succeeded;
}

/* Vérifie que la simulation rejette une configuration finale NULL. */
static TestResult test_simulation_rejects_null_configuration(void) {
  /* Given */
  const FinalConfig *final = NULL;

  /* When */
  bool simulation_succeeded = run_simulation(final, NULL);

  /* Then */
  return assert_condition(!simulation_succeeded,
                          "simulation rejects a null configuration");
}

/* Vérifie que la simulation rejette une période contenant une date invalide. */
static TestResult test_simulation_rejects_invalid_configuration(void) {
  /* Given */
  const FinalConfig final = {
      .jour_debut = 31,
      .mois_debut = 4,
      .annee_debut = 2026,
      .jour_fin = 2,
      .mois_fin = 5,
      .annee_fin = 2026,
      .latitude = 45.0,
      .mode_declinaison = MODE_SPENCER,
      .declination_func = calculate_spencer_declination,
  };

  /* When */
  bool simulation_succeeded = run_simulation(&final, NULL);

  /* Then */
  return assert_condition(!simulation_succeeded,
                          "simulation rejects an invalid date range");
}

/* Cas de test */

/* Vérifie que la simulation traite toutes les dates, bornes incluses. */
static TestResult test_simulation_includes_both_range_endpoints(void) {
  /* Given */
  FinalConfig final = {31,
                       12,
                       2025,
                       2,
                       1,
                       2026,
                       45.0,
                       MODE_SPENCER,
                       calculate_spencer_declination,
                       0.0};
  StdoutCapture capture;
  char output[1024];
  const char *expected_start_date = "31/12/2025";
  const char *expected_middle_date = "01/01/2026";
  const char *expected_end_date = "02/01/2026";
  if (!start_stdout_capture(&capture)) {
    return assert_condition(0, "simulation output capture starts");
  }

  /* When */
  run_simulation(&final, NULL);

  /* Then */
  if (!finish_stdout_capture(&capture, output, sizeof(output))) {
    return assert_condition(0, "simulation output capture completes");
  }
  TestResult passed =
      assert_condition(strstr(output, expected_start_date) != NULL,
                       "simulation processes the start date");
  if (assert_condition(strstr(output, expected_middle_date) != NULL,
                       "simulation advances into the next year") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  if (assert_condition(strstr(output, expected_end_date) != NULL,
                       "simulation processes the end date inclusively") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  return passed;
}

/* Vérifie que l'arrondi à 60 secondes entraîne une retenue sur la minute suivante. */
static TestResult test_simulation_carries_rounded_seconds(void) {
  /* Given */
  const double pi = acos(-1.0);
  const double target_duration_hours = 12.0 + 34.0 / 60.0 + 59.75 / 3600.0;
  const SolarDate solar_date = {.jour_ordinal = 1, .annee = 2026};
  const double declination =
      calculate_spencer_declination(solar_date) * pi / 180.0;
  const double target_hour_angle = target_duration_hours * pi / 24.0;
  FinalConfig final = {
      .jour_debut = 1,
      .mois_debut = 1,
      .annee_debut = 2026,
      .jour_fin = 1,
      .mois_fin = 1,
      .annee_fin = 2026,
      .latitude = 0.0,
      .mode_declinaison = MODE_SPENCER,
      .declination_func = calculate_spencer_declination,
      .altitude_solaire_seuil_degres =
          asin(cos(target_hour_angle) * cos(declination)) * 180.0 / pi,
  };
  StdoutCapture capture;
  char output[512];
  const char *expected_duration = "-> 12:35:00";
  if (!start_stdout_capture(&capture)) {
    return assert_condition(0, "simulation output capture starts");
  }

  /* When */
  run_simulation(&final, NULL);

  /* Then */
  if (!finish_stdout_capture(&capture, output, sizeof(output))) {
    return assert_condition(0, "simulation output capture completes");
  }
  return assert_condition(strstr(output, expected_duration) != NULL,
                          "rounded seconds carry into the next minute");
}

/* Vérifie que l'export CSV contient une ligne par date de la période. */
static TestResult test_simulation_writes_csv_rows_for_each_day(void) {
  /* Given */
  FinalConfig final = {31,
                       12,
                       2025,
                       2,
                       1,
                       2026,
                       45.0,
                       MODE_SPENCER,
                       calculate_spencer_declination,
                       0.0};
  FILE *csv_output = tmpfile();
  char output[512];
  if (!csv_output) {
    return assert_condition(0, "CSV output stream is created");
  }

  /* When */
  bool simulation_succeeded = run_simulation(&final, csv_output);
  fflush(csv_output);
  rewind(csv_output);
  size_t bytes_read = fread(output, 1, sizeof(output) - 1, csv_output);
  output[bytes_read] = '\0';
  fclose(csv_output);

  /* Then */
  return assert_condition(
      simulation_succeeded &&
        strstr(output,
           "Date,Duration_Hours,Duration_Sinusoidal_Hours,"
           "Duration_Spencer_Hours,Duration_Meeus_Hours,"
           "Solar_Altitude_Threshold_Degrees\n") != NULL &&
          strstr(output, "31/12/2025,") != NULL &&
          strstr(output, "01/01/2026,") != NULL &&
          strstr(output, "02/01/2026,") != NULL,
      "simulation exports a CSV row for each inclusive date");
}

static const TestCase test_cases[] = {
    {"test_simulation_rejects_null_configuration",
     test_simulation_rejects_null_configuration},
    {"test_simulation_rejects_invalid_configuration",
     test_simulation_rejects_invalid_configuration},
    {"test_simulation_includes_both_range_endpoints",
     test_simulation_includes_both_range_endpoints},
    {"test_simulation_carries_rounded_seconds",
     test_simulation_carries_rounded_seconds},
    {"test_simulation_writes_csv_rows_for_each_day",
     test_simulation_writes_csv_rows_for_each_day},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("simulation", test_cases,
                        sizeof(test_cases) / sizeof(test_cases[0]));
}