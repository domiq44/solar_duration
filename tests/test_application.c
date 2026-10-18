#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "test_assertions.h"
#include "test_runner.h"

#define PATH_BUFFER_SIZE 4096
#define OUTPUT_BUFFER_SIZE 8192

typedef enum {
  APPLICATION_WITH_FILE_OPTIONS,
  APPLICATION_WITH_DEFAULTS,
  APPLICATION_WITH_CHECK_ONLY,
  APPLICATION_WITH_INVALID_LOGGER_CONFIG,
  APPLICATION_WITH_WARN_LOGGER_CONFIG,
  APPLICATION_WITH_HELP,
  APPLICATION_WITH_UNKNOWN_OPTION,
  APPLICATION_WITH_MISSING_OPTION_VALUE,
  APPLICATION_WITH_MISSING_CONFIG_FILE,
  APPLICATION_WITH_CSV_OPEN_FAILURE
} ApplicationInvocation;

static const char *application_path;

static int write_text_file(const char *path, const char *contents) {
  FILE *file = fopen(path, "w");
  if (!file) {
    return 0;
  }

  int write_succeeded = fputs(contents, file) >= 0;
  int close_succeeded = fclose(file) == 0;
  return write_succeeded && close_succeeded;
}

static int build_temp_path(char *path, size_t path_size, const char *directory,
                           const char *filename) {
  int length = snprintf(path, path_size, "%s/%s", directory, filename);
  return length >= 0 && (size_t)length < path_size;
}

static int run_application_with_config(const char *config_contents,
                                       char *output, size_t output_size,
                                       char *csv_output, size_t csv_output_size,
                                       int *exit_code,
                                       ApplicationInvocation invocation) {
  char directory_template[] = "/tmp/solar_duration_integration_XXXXXX";
  char *temporary_directory = mkdtemp(directory_template);
  char config_directory[PATH_BUFFER_SIZE];
  char data_path[PATH_BUFFER_SIZE];
  char logger_path[PATH_BUFFER_SIZE];
  char output_path[PATH_BUFFER_SIZE];
  char csv_path[PATH_BUFFER_SIZE];
  char missing_data_path[PATH_BUFFER_SIZE];
  char missing_csv_directory[PATH_BUFFER_SIZE];
  char missing_csv_path[PATH_BUFFER_SIZE];
  char resolved_application_path[PATH_BUFFER_SIZE];
  int execution_succeeded = 0;

  if (!temporary_directory || output_size < 2 ||
      !realpath(application_path, resolved_application_path) ||
      !build_temp_path(config_directory, sizeof(config_directory),
               temporary_directory, "config") ||
      !build_temp_path(data_path, sizeof(data_path), config_directory,
                       "solar_duration.dat") ||
      !build_temp_path(logger_path, sizeof(logger_path), config_directory,
                       "solar_duration.cfg") ||
      !build_temp_path(missing_data_path, sizeof(missing_data_path),
               config_directory, "missing.dat") ||
      !build_temp_path(output_path, sizeof(output_path), temporary_directory,
                       "application.log") ||
      !build_temp_path(csv_path, sizeof(csv_path), temporary_directory,
               "results.csv") ||
      !build_temp_path(missing_csv_directory, sizeof(missing_csv_directory),
               temporary_directory, "missing-output") ||
      !build_temp_path(missing_csv_path, sizeof(missing_csv_path),
               missing_csv_directory, "results.csv")) {
    if (temporary_directory) {
      rmdir(temporary_directory);
    }
    return 0;
  }

  if (mkdir(config_directory, 0700) != 0) {
    goto cleanup;
  }

  const char *logger_contents =
      invocation == APPLICATION_WITH_INVALID_LOGGER_CONFIG
          ? "level = TRACE\n"
          : invocation == APPLICATION_WITH_WARN_LOGGER_CONFIG
                ? "level = WARN\n"
                : "level = INFO\n";
    if (!write_text_file(data_path, config_contents) ||
      !write_text_file(logger_path, logger_contents)) {
    goto cleanup;
  }

  fflush(stdout);
  fflush(stderr);
  pid_t child_process = fork();
  if (child_process < 0) {
    goto cleanup;
  }
  if (child_process == 0) {
    int output_file = open(output_path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (output_file < 0 || chdir(temporary_directory) != 0 ||
        dup2(output_file, STDOUT_FILENO) < 0 ||
        dup2(output_file, STDERR_FILENO) < 0) {
      _exit(126);
    }
    close(output_file);
    if (invocation == APPLICATION_WITH_CHECK_ONLY) {
      execl(resolved_application_path, resolved_application_path, "--check",
            "--config", data_path, "--logger-config", logger_path,
            (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_INVALID_LOGGER_CONFIG) {
      execl(resolved_application_path, resolved_application_path, "--check",
            "--config", data_path, "--logger-config", logger_path,
            (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_DEFAULTS) {
      execl(resolved_application_path, resolved_application_path, (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_HELP) {
      execl(resolved_application_path, resolved_application_path, "--help",
            (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_UNKNOWN_OPTION) {
      execl(resolved_application_path, resolved_application_path, "--unknown",
            "value", (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_MISSING_OPTION_VALUE) {
      execl(resolved_application_path, resolved_application_path, "--config",
            (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_MISSING_CONFIG_FILE) {
      execl(resolved_application_path, resolved_application_path, "--config",
            missing_data_path, "--logger-config", logger_path, (char *)NULL);
    }
    if (invocation == APPLICATION_WITH_CSV_OPEN_FAILURE) {
      execl(resolved_application_path, resolved_application_path, "--config",
            data_path, "--logger-config", logger_path, "--csv",
            missing_csv_path, (char *)NULL);
    }
    execl(resolved_application_path, resolved_application_path, "--config",
          data_path, "--logger-config", logger_path, "--csv", csv_path,
          (char *)NULL);
    _exit(127);
  }

  int child_status;
  while (waitpid(child_process, &child_status, 0) < 0) {
    if (errno != EINTR) {
      goto cleanup;
    }
  }
  if (WIFEXITED(child_status)) {
    *exit_code = WEXITSTATUS(child_status);
  } else {
    *exit_code = -1;
  }

  FILE *output_file = fopen(output_path, "r");
  if (!output_file) {
    goto cleanup;
  }
  size_t bytes_read = fread(output, 1, output_size - 1, output_file);
  int read_succeeded = !ferror(output_file);
  output[bytes_read] = '\0';
  int close_succeeded = fclose(output_file) == 0;
  execution_succeeded = read_succeeded && close_succeeded;

  FILE *csv_file = fopen(csv_path, "r");
  csv_output[0] = '\0';
  if (csv_file) {
    size_t csv_bytes_read = fread(csv_output, 1, csv_output_size - 1, csv_file);
    int csv_read_succeeded = !ferror(csv_file);
    csv_output[csv_bytes_read] = '\0';
    int csv_close_succeeded = fclose(csv_file) == 0;
    execution_succeeded =
        execution_succeeded && csv_read_succeeded && csv_close_succeeded;
  }

cleanup:
  unlink(data_path);
  unlink(logger_path);
  unlink(output_path);
  unlink(csv_path);
  rmdir(config_directory);
  rmdir(temporary_directory);
  return execution_succeeded;
}

/* Vérifie que l'interface en ligne de commande traite une période sur deux années et exporte chaque date. */
static TestResult test_application_runs_cross_year_period(void) {
  /* Given */
  const char *config_contents = "jour_debut = 31\n"
                                "mois_debut = 12\n"
                                "annee_debut = 2025\n"
                                "jour_fin = 2\n"
                                "mois_fin = 1\n"
                                "annee_fin = 2026\n"
                                "latitude = 48 deg 51 min 24 sec Nord\n"
                                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;
  int execution_succeeded;

  /* When */
  execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_FILE_OPTIONS);
  int completed_successfully = exit_code == 0;

  /* Then */
  return assert_condition(
      execution_succeeded && completed_successfully &&
          strstr(output, "Période: 31/12/2025 au 02/01/2026") != NULL &&
          strstr(output, "31/12/2025") != NULL &&
          strstr(output, "01/01/2026") != NULL &&
          strstr(output, "02/01/2026") != NULL &&
          strstr(output, "Seuil d'altitude solaire: -0.833°") != NULL &&
             strstr(csv_output,
               "Date,Duration_Hours,Duration_Sinusoidal_Hours,"
            "Duration_Spencer_Hours,Duration_Meeus_Hours,"
            "Solar_Altitude_Threshold_Degrees\n") != NULL &&
          strstr(csv_output, "31/12/2025,") != NULL &&
          strstr(csv_output, "01/01/2026,") != NULL &&
          strstr(csv_output, "02/01/2026,") != NULL &&
          strstr(output, "SIMULATION TERMINÉE AVEC SUCCÈS") != NULL,
      "application accepts file options and exports the inclusive date range");
}

/* Vérifie que l'application rejette une date de fin impossible. */
static TestResult test_application_rejects_invalid_end_date(void) {
  /* Given */
  const char *config_contents = "jour_debut = 30\n"
                                "mois_debut = 4\n"
                                "annee_debut = 2026\n"
                                "jour_fin = 31\n"
                                "mois_fin = 4\n"
                                "annee_fin = 2026\n"
                                "latitude = 48 deg 51 min 24 sec Nord\n"
                                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;
  int execution_succeeded;

  /* When */
  execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_FILE_OPTIONS);
  int rejected_with_error = exit_code != 0;

  /* Then */
  return assert_condition(
      execution_succeeded && rejected_with_error &&
          strstr(output, "Date de fin (31/04/2026)") != NULL &&
          strstr(output, "SIMULATION TERMINÉE AVEC SUCCÈS") == NULL,
      "application rejects an impossible date before starting simulation");
}

/* Vérifie que l'application utilise les chemins de configuration par défaut. */
static TestResult test_application_runs_with_default_config_paths(void) {
  /* Given */
  const char *config_contents = "jour_debut = 1\n"
                                "mois_debut = 1\n"
                                "annee_debut = 2026\n"
                                "jour_fin = 1\n"
                                "mois_fin = 1\n"
                                "annee_fin = 2026\n"
                                "latitude = 48 deg 51 min 24 sec Nord\n"
                                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_DEFAULTS);

  /* Then */
  return assert_condition(
      execution_succeeded && exit_code == 0 &&
          strstr(output, "Période: 01/01/2026 au 01/01/2026") != NULL &&
          strstr(output, "SIMULATION TERMINÉE AVEC SUCCÈS") != NULL,
      "application loads default config paths when no options are provided");
}

/* Vérifie que les résultats quotidiens restent visibles avec un niveau WARN. */
static TestResult test_application_shows_results_with_warn_log_level(void) {
  /* Given */
  const char *config_contents = "jour_debut = 1\n"
                "mois_debut = 1\n"
                "annee_debut = 2026\n"
                "jour_fin = 1\n"
                "mois_fin = 1\n"
                "annee_fin = 2026\n"
                "latitude = 48 deg 51 min 24 sec Nord\n"
                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
    config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
    &exit_code, APPLICATION_WITH_WARN_LOGGER_CONFIG);

  /* Then */
  return assert_condition(
    execution_succeeded && exit_code == 0 &&
      strstr(output, "[RESULT]: 01/01/2026") != NULL &&
      strstr(output, "SIMULATION TERMINÉE AVEC SUCCÈS") == NULL,
    "daily results remain visible when INFO logs are filtered");
}

/* Vérifie que l'option de validation contrôle la configuration sans simuler ni exporter. */
static TestResult
test_application_check_only_validates_without_simulating(void) {
  /* Given */
  const char *config_contents = "jour_debut = 1\n"
                                "mois_debut = 1\n"
                                "annee_debut = 2026\n"
                                "jour_fin = 2\n"
                                "mois_fin = 1\n"
                                "annee_fin = 2026\n"
                                "latitude = 48 deg 51 min 24 sec Nord\n"
                                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_CHECK_ONLY);

  /* Then */
  return assert_condition(
      execution_succeeded && exit_code == 0 &&
          strstr(output, "Configuration valide") != NULL &&
          strstr(output, "Démarrage de l'itération") == NULL &&
          strstr(output, "SIMULATION TERMINÉE AVEC SUCCÈS") == NULL &&
          csv_output[0] == '\0',
      "check option validates configuration without simulating or exporting");
}

    /* Vérifie que l'application continue avec INFO si la configuration du logger est invalide. */
    static TestResult test_application_uses_info_when_logger_config_is_invalid(void) {
      /* Given */
      const char *config_contents = "jour_debut = 1\n"
                    "mois_debut = 1\n"
                    "annee_debut = 2026\n"
                    "jour_fin = 1\n"
                    "mois_fin = 1\n"
                    "annee_fin = 2026\n"
                    "latitude = 48 deg 51 min 24 sec Nord\n"
                    "mode_solaire = 2\n";
      char output[OUTPUT_BUFFER_SIZE];
      char csv_output[OUTPUT_BUFFER_SIZE];
      int exit_code = -1;

      /* When */
      int execution_succeeded = run_application_with_config(
        config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
        &exit_code, APPLICATION_WITH_INVALID_LOGGER_CONFIG);

      /* Then */
      return assert_condition(
        execution_succeeded && exit_code == 0 &&
          strstr(output, "poursuite avec le niveau INFO") != NULL &&
          strstr(output, "Configuration valide") != NULL,
        "application continues with INFO after invalid logger configuration");
    }

/* Vérifie que l'option de validation rejette une configuration invalide sans simuler. */
static TestResult
test_application_check_only_rejects_invalid_configuration(void) {
  /* Given */
  const char *config_contents = "jour_debut = 30\n"
                                "mois_debut = 4\n"
                                "annee_debut = 2026\n"
                                "jour_fin = 31\n"
                                "mois_fin = 4\n"
                                "annee_fin = 2026\n"
                                "latitude = 48 deg 51 min 24 sec Nord\n"
                                "mode_solaire = 2\n";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_CHECK_ONLY);

  /* Then */
  return assert_condition(
      execution_succeeded && exit_code != 0 &&
          strstr(output, "Date de fin (31/04/2026)") != NULL &&
          strstr(output, "Configuration valide") == NULL &&
          csv_output[0] == '\0',
      "check option rejects invalid configuration without simulating");
}

/* Vérifie que l'option d'aide affiche l'utilisation et se termine avec succès. */
static TestResult test_application_help_exits_successfully(void) {
  /* Given */
  const char *config_contents = "";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_HELP);

  /* Then */
  return assert_condition(execution_succeeded && exit_code == 0 &&
                              strstr(output, "Usage:") != NULL &&
                              strstr(output, "--logger-config") != NULL &&
                              strstr(output, "--help") != NULL,
                          "help option prints usage and exits successfully");
}

/* Vérifie qu'une option inconnue de la ligne de commande est rejetée. */
static TestResult test_application_rejects_unknown_option(void) {
  /* Given */
  const char *config_contents = "";
  char output[OUTPUT_BUFFER_SIZE];
  char csv_output[OUTPUT_BUFFER_SIZE];
  int exit_code = -1;

  /* When */
  int execution_succeeded = run_application_with_config(
      config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
      &exit_code, APPLICATION_WITH_UNKNOWN_OPTION);

  /* Then */
  return assert_condition(execution_succeeded && exit_code == 2 &&
                              strstr(output, "Usage:") != NULL,
                          "unknown command-line option is rejected with usage");
}

              /* Vérifie qu'une option sans valeur affiche l'usage et échoue. */
              static TestResult test_application_rejects_option_without_value(void) {
                /* Given */
                const char *config_contents = "";
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                int execution_succeeded = run_application_with_config(
                  config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
                  &exit_code, APPLICATION_WITH_MISSING_OPTION_VALUE);

                /* Then */
                return assert_condition(execution_succeeded && exit_code == 2 &&
                              strstr(output, "Usage:") != NULL,
                            "option without a value is rejected with usage");
              }

              /* Vérifie que l'application rapporte une configuration absente. */
              static TestResult test_application_reports_missing_config_file(void) {
                /* Given */
                const char *config_contents = "";
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                int execution_succeeded = run_application_with_config(
                  config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
                  &exit_code, APPLICATION_WITH_MISSING_CONFIG_FILE);

                /* Then */
                return assert_condition(execution_succeeded && exit_code != 0 &&
                              strstr(output, "Impossible de trouver") != NULL,
                            "missing config file is reported to the user");
              }

              /* Vérifie que l'application traite une configuration incomplète. */
              static TestResult test_application_reports_incomplete_config(void) {
                /* Given */
                const char *config_contents = "jour_debut = 1\n";
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                int execution_succeeded = run_application_with_config(
                  config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
                  &exit_code, APPLICATION_WITH_FILE_OPTIONS);

                /* Then */
                return assert_condition(execution_succeeded && exit_code != 0 &&
                              strstr(output, "Configuration incomplète") != NULL,
                            "incomplete config is reported to the user");
              }

              /* Vérifie que l'application rapporte une valeur de configuration non convertible. */
              static TestResult test_application_reports_config_conversion_failure(void) {
                /* Given */
                const char *config_contents = "jour_debut = 1\n"
                              "mois_debut = 1\n"
                              "annee_debut = 2026\n"
                              "jour_fin = 1\n"
                              "mois_fin = 1\n"
                              "annee_fin = 2026\n"
                              "latitude = 48 deg 51 min 24 sec Nord\n"
                              "mode_solaire = invalide\n";
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                int execution_succeeded = run_application_with_config(
                  config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
                  &exit_code, APPLICATION_WITH_FILE_OPTIONS);

                /* Then */
                return assert_condition(execution_succeeded && exit_code != 0 &&
                              strstr(output, "Erreur de conversion") != NULL,
                            "invalid config value is reported as conversion failure");
              }

              /* Vérifie que l'application échoue proprement si le CSV ne peut être créé. */
              static TestResult test_application_reports_csv_open_failure(void) {
                /* Given */
                const char *config_contents = "jour_debut = 1\n"
                              "mois_debut = 1\n"
                              "annee_debut = 2026\n"
                              "jour_fin = 1\n"
                              "mois_fin = 1\n"
                              "annee_fin = 2026\n"
                              "latitude = 48 deg 51 min 24 sec Nord\n"
                              "mode_solaire = 2\n";
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                int execution_succeeded = run_application_with_config(
                  config_contents, output, sizeof(output), csv_output, sizeof(csv_output),
                  &exit_code, APPLICATION_WITH_CSV_OPEN_FAILURE);

                /* Then */
                return assert_condition(execution_succeeded && exit_code != 0 &&
                              strstr(output, "Impossible de créer le fichier CSV") != NULL,
                            "CSV creation failure is reported to the user");
              }

              /* Vérifie que les deux autres modèles solaires sont annoncés et simulés. */
              static TestResult test_application_runs_sinusoidal_and_meeus_models(void) {
                /* Given */
                const char *model_configs[] = {
                  "jour_debut = 1\nmois_debut = 1\nannee_debut = 2026\n"
                  "jour_fin = 1\nmois_fin = 1\nannee_fin = 2026\n"
                  "latitude = 48 deg 51 min 24 sec Nord\nmode_solaire = 1\n",
                  "jour_debut = 1\nmois_debut = 1\nannee_debut = 2026\n"
                  "jour_fin = 1\nmois_fin = 1\nannee_fin = 2026\n"
                  "latitude = 48 deg 51 min 24 sec Nord\nmode_solaire = 3\n"};
                const char *expected_model_messages[] = {
                  "Utilisation du Mode SINUSOIDAL.", "Utilisation du Mode MEEUS."};
                char output[OUTPUT_BUFFER_SIZE];
                char csv_output[OUTPUT_BUFFER_SIZE];
                int exit_code = -1;

                /* When */
                for (size_t index = 0; index < 2; index++) {
                int execution_succeeded = run_application_with_config(
                  model_configs[index], output, sizeof(output), csv_output,
                  sizeof(csv_output), &exit_code, APPLICATION_WITH_FILE_OPTIONS);
                if (!execution_succeeded || exit_code != 0 ||
                  strstr(output, expected_model_messages[index]) == NULL ||
                  strstr(csv_output, "01/01/2026,") == NULL) {
                  return assert_condition(0,
                              "application runs the selected solar model");
                }
                }

                /* Then */
                return assert_condition(1, "application runs the selected solar models");
              }

static const TestCase test_cases[] = {
    {"test_application_runs_cross_year_period",
     test_application_runs_cross_year_period},
    {"test_application_rejects_invalid_end_date",
     test_application_rejects_invalid_end_date},
    {"test_application_runs_with_default_config_paths",
     test_application_runs_with_default_config_paths},
    {"test_application_shows_results_with_warn_log_level",
     test_application_shows_results_with_warn_log_level},
    {"test_application_check_only_validates_without_simulating",
     test_application_check_only_validates_without_simulating},
    {"test_application_uses_info_when_logger_config_is_invalid",
     test_application_uses_info_when_logger_config_is_invalid},
    {"test_application_check_only_rejects_invalid_configuration",
     test_application_check_only_rejects_invalid_configuration},
    {"test_application_help_exits_successfully",
     test_application_help_exits_successfully},
    {"test_application_rejects_unknown_option",
     test_application_rejects_unknown_option},
    {"test_application_rejects_option_without_value",
     test_application_rejects_option_without_value},
    {"test_application_reports_missing_config_file",
     test_application_reports_missing_config_file},
    {"test_application_reports_incomplete_config",
     test_application_reports_incomplete_config},
    {"test_application_reports_config_conversion_failure",
     test_application_reports_config_conversion_failure},
    {"test_application_reports_csv_open_failure",
     test_application_reports_csv_open_failure},
    {"test_application_runs_sinusoidal_and_meeus_models",
     test_application_runs_sinusoidal_and_meeus_models},
};

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <application-path>\n", argv[0]);
    return 2;
  }

  application_path = argv[1];
  return run_test_suite("application integration", test_cases,
                        sizeof(test_cases) / sizeof(test_cases[0]));
}