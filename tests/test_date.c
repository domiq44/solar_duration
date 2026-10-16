#include "date.h"
#include "test_assertions.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

/* Cas de test */

/* Vérifie qu'une année divisible par quatre est généralement bissextile. */
static TestResult test_2024_is_a_leap_year(void) {
  /* Given */
  const int year = 2024;
  const int expected_is_leap = 1;

  /* When */
  int is_leap = is_leap_year(year);

  /* Then */
  return assert_condition(is_leap == expected_is_leap,
                          "2024 is classified as a leap year");
}

/* Vérifie qu'une année non divisible par quatre n'est pas bissextile. */
static TestResult test_2023_is_not_a_leap_year(void) {
  /* Given */
  const int year = 2023;
  const int expected_is_leap = 0;

  /* When */
  int is_leap = is_leap_year(year);

  /* Then */
  return assert_condition(is_leap == expected_is_leap,
                          "2023 is classified as a common year");
}

/* Vérifie la règle grégorienne des années séculaires bissextiles. */
static TestResult test_gregorian_century_leap_year_rules(void) {
  /* Given */
  const int expected_1900_is_leap = 0;
  const int expected_2000_is_leap = 1;

  /* When */
  int is_1900_leap = is_leap_year(1900);
  int is_2000_leap = is_leap_year(2000);

  /* Then */
  return assert_condition(is_1900_leap == expected_1900_is_leap &&
                              is_2000_leap == expected_2000_is_leap,
                          "Gregorian century years follow the 400-year rule");
}

/* Vérifie que des composantes de mois ou de jour invalides n'ont pas de rang ordinal. */
static TestResult test_ordinal_rejects_invalid_calendar_components(void) {
  /* Given */
  const int expected_invalid_ordinal = -1;

  /* When */
  int invalid_components_are_rejected =
      date_to_ordinal((CalendarDate){1, 0, 2026}) == expected_invalid_ordinal &&
      date_to_ordinal((CalendarDate){0, 1, 2026}) == expected_invalid_ordinal &&
      date_to_ordinal((CalendarDate){31, 4, 2026}) == expected_invalid_ordinal;

  /* Then */
  return assert_condition(invalid_components_are_rejected,
                          "out-of-range month and day values are rejected");
}

/* Vérifie que le jour bissextile correspond au 60e jour d'une année bissextile. */
static TestResult test_leap_day_has_ordinal_sixty(void) {
  /* Given */
  const int expected_ordinal = 60;

  /* When */
  int ordinal = date_to_ordinal((CalendarDate){29, 2, 2024});

  /* Then */
  return assert_condition(ordinal == expected_ordinal,
                          "February 29 has ordinal 60 in 2024");
}

/* Vérifie que le 29 février est invalide lors d'une année commune. */
static TestResult test_february_twenty_nine_is_invalid_in_2023(void) {
  /* Given */
  const CalendarDate date = {29, 2, 2023};
  const int expected_valid = 0;

  /* When */
  int valid = is_date_valid(date);

  /* Then */
  return assert_condition(valid == expected_valid,
                          "February 29 is invalid in 2023");
}

/* Vérifie que le lendemain du 28 février est le jour bissextile si l'année l'est. */
static TestResult test_advance_day_enters_leap_day(void) {
  /* Given */
  CalendarDate date = {28, 2, 2024};
  const CalendarDate expected_date = {29, 2, 2024};

  /* When */
  advance_day(&date);

  /* Then */
  return assert_condition(date.jour == expected_date.jour &&
                              date.mois == expected_date.mois &&
                              date.annee == expected_date.annee,
                          "February 28 advances to leap day");
}

/* Vérifie que le jour suivant le 31 décembre appartient à l'année suivante. */
static TestResult test_advance_day_rolls_over_to_next_year(void) {
  /* Given */
  CalendarDate date = {31, 12, 2024};
  const CalendarDate expected_date = {1, 1, 2025};

  /* When */
  advance_day(&date);

  /* Then */
  return assert_condition(date.jour == expected_date.jour &&
                              date.mois == expected_date.mois &&
                              date.annee == expected_date.annee,
                          "December 31 advances to January 1 of next year");
}

static const TestCase test_cases[] = {
    {"test_2024_is_a_leap_year", test_2024_is_a_leap_year},
    {"test_2023_is_not_a_leap_year", test_2023_is_not_a_leap_year},
    {"test_gregorian_century_leap_year_rules",
     test_gregorian_century_leap_year_rules},
    {"test_ordinal_rejects_invalid_calendar_components",
     test_ordinal_rejects_invalid_calendar_components},
    {"test_leap_day_has_ordinal_sixty", test_leap_day_has_ordinal_sixty},
    {"test_february_twenty_nine_is_invalid_in_2023",
     test_february_twenty_nine_is_invalid_in_2023},
    {"test_advance_day_enters_leap_day", test_advance_day_enters_leap_day},
    {"test_advance_day_rolls_over_to_next_year",
     test_advance_day_rolls_over_to_next_year},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("date", test_cases,
                       sizeof(test_cases) / sizeof(test_cases[0]));
}