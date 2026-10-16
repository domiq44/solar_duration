#include "logger.h"
#include "test_assertions.h"
#include "test_file_utils.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

/* Cas de test */

/* Vérifie qu'une configuration valide applique le niveau de journalisation demandé. */
static TestResult test_logger_init_loads_configured_level(void) {
  /* Given */
  char path[64];
  const char *contents = "# logger config\n  level = DEBUG  \n";
  const int expected_initialized = 1;
  const LogLevel expected_level = LOG_DEBUG;
  if (!create_temporary_file(path, sizeof(path), contents)) {
    return assert_condition(0, "temporary logger configuration is created");
  }
  current_log_level = LOG_INFO;

  /* When */
  int initialized = logger_init(path);

  /* Then */
  TestResult passed = assert_condition(initialized == expected_initialized,
                                "logger initializes from a valid config file");
  if (assert_condition(current_log_level == expected_level,
                       "configured DEBUG level is applied") == TEST_FAILED) {
    passed = TEST_FAILED;
  }
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que l'initialisation du journal signale un fichier de configuration absent. */
static TestResult test_logger_init_rejects_missing_config_file(void) {
  /* Given */
  char path[64];
  const int expected_initialized = 0;
  const LogLevel expected_level = LOG_INFO;
  if (!create_temporary_file(path, sizeof(path), "")) {
    return assert_condition(0, "temporary logger path is created");
  }
  remove_temporary_file(path);
  current_log_level = LOG_DEBUG;

  /* When */
  int initialized = logger_init(path);

  /* Then */
  TestResult passed = assert_condition(initialized == expected_initialized,
                                       "logger reports a missing config file");
  if (assert_condition(current_log_level == expected_level,
                       "missing config falls back to INFO") == TEST_FAILED) {
    passed = TEST_FAILED;
  }
  return passed;
}

/* Vérifie qu'un niveau non pris en charge déclenche le niveau INFO par défaut. */
static TestResult test_logger_init_rejects_unknown_level(void) {
  /* Given */
  char path[64];
  const char *contents = "level = TRACE\n";
  const int expected_initialized = 0;
  const LogLevel expected_level = LOG_INFO;
  if (!create_temporary_file(path, sizeof(path), contents)) {
    return assert_condition(0, "temporary logger configuration is created");
  }
  current_log_level = LOG_DEBUG;

  /* When */
  int initialized = logger_init(path);

  /* Then */
  TestResult passed = assert_condition(initialized == expected_initialized,
                                "unknown logger level is rejected");
  if (assert_condition(current_log_level == expected_level,
                       "invalid level falls back to INFO") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  remove_temporary_file(path);
  return passed;
}

/* Vérifie qu'une configuration sans clé reconnue est rejetée. */
static TestResult test_logger_init_rejects_unknown_key(void) {
  /* Given */
  char path[64];
  const char *contents = "verbosity = DEBUG\n";
  const int expected_initialized = 0;
  const LogLevel expected_level = LOG_INFO;
  if (!create_temporary_file(path, sizeof(path), contents)) {
    return assert_condition(0, "temporary logger configuration is created");
  }
  current_log_level = LOG_INFO;

  /* When */
  int initialized = logger_init(path);

  /* Then */
  TestResult passed = assert_condition(initialized == expected_initialized,
                                       "unknown logger key is rejected");
  if (assert_condition(current_log_level == expected_level,
                       "unknown key leaves the current level unchanged") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  remove_temporary_file(path);
  return passed;
}

/* Vérifie que le niveau INFO peut être chargé depuis la configuration du journal. */
static TestResult test_logger_init_loads_info_level(void) {
  /* Given */
  char path[64];
  const char *contents = "level = INFO\n";
  const LogLevel expected_level = LOG_INFO;
  if (!create_temporary_file(path, sizeof(path), contents)) {
    return assert_condition(0, "temporary logger configuration is created");
  }
  current_log_level = LOG_DEBUG;

  /* When */
  int initialized = logger_init(path);

  /* Then */
  TestResult passed = assert_condition(initialized,
                                "logger initializes from an INFO configuration");
  if (assert_condition(current_log_level == expected_level,
                       "configured INFO level is applied") == TEST_FAILED) {
    passed = TEST_FAILED;
  }
  remove_temporary_file(path);
  return passed;
}

/* Vérifie qu'un message critique demande à l'appelant d'interrompre le traitement. */
static TestResult test_critical_log_requests_application_stop(void) {
  /* Given */
  const int expected_stop_status = 1;

  /* When */
  int stop_status = log_critical("critical test message");

  /* Then */
  return assert_condition(stop_status == expected_stop_status,
                          "critical log signals the caller to stop");
}

static const TestCase test_cases[] = {
    {"test_logger_init_loads_configured_level",
     test_logger_init_loads_configured_level},
    {"test_logger_init_rejects_missing_config_file",
     test_logger_init_rejects_missing_config_file},
    {"test_logger_init_rejects_unknown_level",
     test_logger_init_rejects_unknown_level},
    {"test_logger_init_rejects_unknown_key",
     test_logger_init_rejects_unknown_key},
    {"test_logger_init_loads_info_level", test_logger_init_loads_info_level},
    {"test_critical_log_requests_application_stop",
     test_critical_log_requests_application_stop},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("logger", test_cases,
                       sizeof(test_cases) / sizeof(test_cases[0]));
}