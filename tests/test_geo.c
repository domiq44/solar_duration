#include <math.h>
#include <string.h>

#include "geo.h"
#include "test_assertions.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

/* Cas de test */

/* Vérifie que les fractions de seconde de latitude conservent leur précision. */
static TestResult test_fractional_seconds_are_preserved(void) {
  /* Given */
  const double expected_latitude = 47.0 + 17.0 / 60.0 + 48.5 / 3600.0;
  double latitude = 0.0;
  int parse_status;

  /* When */
  parse_status =
      parse_latitude_string("47 deg 17 min 48.5 sec Nord", &latitude);

  /* Then */
  TestResult passed = assert_condition(
      parse_status == ERR_OK, "fractional-seconds latitude is accepted");
  if (assert_condition(fabs(latitude - expected_latitude) < 1e-9,
                       "fractional seconds retain their precision") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  return passed;
}

/* Vérifie que les secondes de latitude non finies sont rejetées. */
static TestResult test_non_finite_seconds_are_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status =
      parse_latitude_string("47 deg 17 min nan sec Nord", &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "non-finite latitude seconds are rejected");
}

/* Vérifie qu'un suffixe d'hémisphère non pris en charge est rejeté. */
static TestResult test_invalid_hemisphere_is_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status =
      parse_latitude_string("47 deg 17 min 48 sec Est", &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "unknown latitude hemisphere is rejected");
}

/* Vérifie qu'une syntaxe degrés-minutes-secondes mal formée est rejetée. */
static TestResult test_invalid_coordinate_syntax_is_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status =
      parse_latitude_string("47 degrees 17 min 48 sec Nord", &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "malformed coordinate syntax is rejected");
}

/* Vérifie qu'une chaîne de latitude NULL est rejetée. */
static TestResult test_null_latitude_string_is_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status = parse_latitude_string(NULL, &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "null latitude string is rejected");
}

/* Vérifie qu'un pointeur de sortie de latitude NULL est rejeté. */
static TestResult test_null_latitude_result_is_rejected(void) {
  /* Given */
  const char *latitude_string = "47 deg 0 min 0 sec Nord";
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status = parse_latitude_string(latitude_string, NULL);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "null latitude result pointer is rejected");
}

/* Vérifie que les coordonnées sud sont converties en valeurs négatives. */
static TestResult test_southern_latitude_is_negative(void) {
  /* Given */
  const double expected_latitude = -(47.0 + 17.0 / 60.0 + 48.0 / 3600.0);
  double latitude = 0.0;
  int parse_status;

  /* When */
  parse_status = parse_latitude_string("47 deg 17 min 48 sec Sud", &latitude);

  /* Then */
  TestResult passed =
      assert_condition(parse_status == ERR_OK, "southern latitude is accepted");
  if (assert_condition(latitude == expected_latitude,
                       "southern latitude has the expected negative value") ==
      TEST_FAILED) {
    passed = TEST_FAILED;
  }
  return passed;
}

/* Vérifie qu'une coordonnée au-delà de 90 degrés est rejetée. */
static TestResult test_latitude_above_ninety_degrees_is_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status =
      parse_latitude_string("120 deg 0 min 0 sec Nord", &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "latitude above 90 degrees is rejected");
}

/* Vérifie le format d'affichage standard en degrés, minutes et secondes. */
static TestResult test_latitude_formatting_is_stable(void) {
  /* Given */
  char buffer[64];
  const char *expected = "47° 17′ 48″ N";

  /* When */
  format_latitude(47.2966666667, buffer, sizeof(buffer));

  /* Then */
  return assert_condition(strcmp(buffer, expected) == 0,
                          "latitude is formatted as degrees, minutes, seconds");
}

/* Vérifie que l'arrondi des secondes se répercute sur les minutes puis les degrés. */
static TestResult test_latitude_formatting_carries_rounded_seconds(void) {
  /* Given */
  char buffer[64];
  const char *expected = "13° 0′ 0″ N";

  /* When */
  format_latitude(12.9999999, buffer, sizeof(buffer));

  /* Then */
  return assert_condition(strcmp(buffer, expected) == 0,
                          "rounded seconds and minutes carry into degrees");
}

/* Vérifie qu'un texte inattendu après l'hémisphère est rejeté. */
static TestResult test_trailing_coordinate_text_is_rejected(void) {
  /* Given */
  double latitude = 0.0;
  const int expected_status = ERR_CONVERSION;

  /* When */
  int parse_status =
      parse_latitude_string("47 deg 17 min 48 sec junk Nord", &latitude);

  /* Then */
  return assert_condition(parse_status == expected_status,
                          "text after latitude seconds is rejected");
}

              static const TestCase test_cases[] = {
    {"test_fractional_seconds_are_preserved",
     test_fractional_seconds_are_preserved},
    {"test_southern_latitude_is_negative", test_southern_latitude_is_negative},
    {"test_latitude_above_ninety_degrees_is_rejected",
     test_latitude_above_ninety_degrees_is_rejected},
    {"test_non_finite_seconds_are_rejected",
     test_non_finite_seconds_are_rejected},
    {"test_invalid_hemisphere_is_rejected",
     test_invalid_hemisphere_is_rejected},
    {"test_invalid_coordinate_syntax_is_rejected",
     test_invalid_coordinate_syntax_is_rejected},
    {"test_null_latitude_string_is_rejected",
     test_null_latitude_string_is_rejected},
    {"test_null_latitude_result_is_rejected",
     test_null_latitude_result_is_rejected},
    {"test_latitude_formatting_is_stable", test_latitude_formatting_is_stable},
    {"test_latitude_formatting_carries_rounded_seconds",
     test_latitude_formatting_carries_rounded_seconds},
    {"test_trailing_coordinate_text_is_rejected",
     test_trailing_coordinate_text_is_rejected},
  };
/* Exécuteur */

int main(void) {
  return run_test_suite("geo", test_cases,
                        sizeof(test_cases) / sizeof(test_cases[0]));
}