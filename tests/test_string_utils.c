#include <string.h>

#include "string_utils.h"
#include "test_assertions.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

/* Cas de test */

/* Vérifie que les espaces en début et en fin de chaîne sont supprimés. */
static TestResult test_strtrim_removes_leading_and_trailing_whitespace(void) {
  /* Given */
  char text[] = " \tSolar Duration\n ";
  const char *expected = "Solar Duration";

  /* When */
  const char *trimmed = strtrim(text);

  /* Then */
  return assert_condition(strcmp(trimmed, expected) == 0,
                          "leading and trailing whitespace is removed");
}

/* Vérifie qu'une entrée composée uniquement d'espaces devient une chaîne vide. */
static TestResult test_strtrim_returns_empty_string_for_whitespace_only_input(void) {
  /* Given */
  char text[] = " \t\n ";
  const char *expected = "";

  /* When */
  const char *trimmed = strtrim(text);

  /* Then */
  return assert_condition(strcmp(trimmed, expected) == 0,
                          "whitespace-only input becomes empty");
}

/* Vérifie qu'une entrée sans espaces superflus est renvoyée sans modification. */
static TestResult test_strtrim_preserves_unpadded_text(void) {
  /* Given */
  char text[] = "Solar";
  const char *expected = "Solar";

  /* When */
  const char *trimmed = strtrim(text);

  /* Then */
  return assert_condition(trimmed == text && strcmp(trimmed, expected) == 0,
                          "unpadded input is returned unchanged");
}

static const TestCase test_cases[] = {
    {"test_strtrim_removes_leading_and_trailing_whitespace",
     test_strtrim_removes_leading_and_trailing_whitespace},
    {"test_strtrim_returns_empty_string_for_whitespace_only_input",
     test_strtrim_returns_empty_string_for_whitespace_only_input},
    {"test_strtrim_preserves_unpadded_text", test_strtrim_preserves_unpadded_text},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("string_utils", test_cases,
                       sizeof(test_cases) / sizeof(test_cases[0]));
}