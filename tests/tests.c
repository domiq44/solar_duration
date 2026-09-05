#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_reader.h"
#include "config_validator.h"
#include "date.h"
#include "geo.h"
#include "solar.h"

static int tests_run = 0;
static int tests_failed = 0;

static void check(int condition, const char *description) {
  tests_run++;
  if (condition) {
    printf("PASS: %s\n", description);
  } else {
    printf("FAIL: %s\n", description);
    tests_failed++;
  }
}

static void test_latitude_parsing(void) {
  double latitude = 0.0;
  const double expected = 47.0 + 17.0 / 60.0 + 48.5 / 3600.0;

  check(parse_latitude_string("47 deg 17 min 48.5 sec Nord", &latitude) ==
            ERR_OK,
        "latitude with fractional seconds is accepted");
  check(fabs(latitude - expected) < 1e-9,
        "latitude with fractional seconds keeps its precision");

  check(parse_latitude_string("47 deg 17 min 48 sec Sud", &latitude) ==
            ERR_OK,
        "southern latitude is accepted");
  check(latitude < 0.0, "southern latitude has a negative sign");

  check(parse_latitude_string("120 deg 0 min 0 sec Nord", &latitude) ==
            ERR_CONVERSION,
        "latitude above 90 degrees is rejected");
}

static void test_dates(void) {
  int day = 28;
  int month = 2;
  int year = 2024;

  check(is_leap_year(2024), "2024 is a leap year");
  check(!is_leap_year(2023), "2023 is not a leap year");
  check(date_to_ordinal(29, 2, 2024) == 60,
        "leap day has ordinal 60");
  check(!is_date_valid(29, 2, 2023), "February 29 is invalid in 2023");

  advance_day(&day, &month, &year);
  check(day == 29 && month == 2 && year == 2024,
        "advance_day handles leap day");

  day = 31;
  month = 12;
  year = 2024;
  advance_day(&day, &month, &year);
  check(day == 1 && month == 1 && year == 2025,
        "advance_day handles year rollover");
}

static RawConfig valid_raw_config(void) {
  RawConfig raw = {(char *)"2026", (char *)"47 deg 17 min 48 sec Nord",
                   (char *)"21", (char *)"8", (char *)"21", (char *)"8",
                   (char *)"2"};
  return raw;
}

static void test_configuration_validation(void) {
  RawConfig raw = valid_raw_config();
  FinalConfig final;

  check(parse_and_validate_config(&raw, &final) == READ_SUCCESS,
        "valid raw configuration is converted");
  check(is_config_fully_valid(&final), "valid final configuration is accepted");
  check(final.declination_func == calculate_spencer_declination,
        "mode 2 selects the Spencer calculation");
  check(is_time_range_valid(21, 8, 22, 8, 2026),
        "chronological date range is accepted");
  check(!is_time_range_valid(22, 8, 21, 8, 2026),
        "inverted date range is rejected");

  raw.raw_mode_solaire_str = (char *)"99";
  check(parse_and_validate_config(&raw, &final) == READ_ERROR_DATA_CONVERSION,
        "unknown solar mode is rejected");

  raw = valid_raw_config();
  raw.raw_mode_solaire_str = NULL;
  check(parse_and_validate_config(&raw, &final) == READ_ERROR_DATA_MISSING,
        "missing configuration field is rejected");
}

static void test_formatting(void) {
  char buffer[64];

  format_latitude(47.2966666667, buffer, sizeof(buffer));
  check(strcmp(buffer, "47° 17′ 48″ N") == 0,
        "latitude formatting produces the expected representation");
}

static void test_meeus_declination(void) {
  double spring_equinox = calculate_meeus_declination(80, 2026);
  double summer_solstice = calculate_meeus_declination(172, 2026);
  double winter_solstice = calculate_meeus_declination(355, 2026);

  check(fabs(spring_equinox) < 1.0,
        "Meeus declination is near zero at the spring equinox");
  check(summer_solstice > 22.0 && summer_solstice < 24.5,
        "Meeus declination is near the summer solstice maximum");
  check(winter_solstice < -22.0 && winter_solstice > -24.5,
        "Meeus declination is near the winter solstice minimum");
}

int main(void) {
  test_latitude_parsing();
  test_dates();
  test_configuration_validation();
  test_formatting();
  test_meeus_declination();

  printf("\n%d tests, %d failures\n", tests_run, tests_failed);
  return tests_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
