#include <math.h>
#include <stdio.h>

#include "date.h"
#include "solar.h"
#include "test_assertions.h"
#include "test_runner.h"

/* Fonctions auxiliaires */

/* Cas de test */

/* Vérifie que le modèle sinusoïdal suit le cycle saisonnier annuel. */
static TestResult test_sinusoidal_declination_tracks_seasonal_cycle(void) {
  /* Given */
  const int year = 2025;
  const double maximum_equinox_declination = 1e-9;

  /* When */
  double spring_declination = calculate_sinusoidal_declination(
      (SolarDate){.jour_ordinal = 81, .annee = year});
  double summer_declination = calculate_sinusoidal_declination(
      (SolarDate){.jour_ordinal = 172, .annee = year});
  double winter_declination = calculate_sinusoidal_declination(
      (SolarDate){.jour_ordinal = 355, .annee = year});

  /* Then */
  return assert_condition(
      fabs(spring_declination) < maximum_equinox_declination &&
          summer_declination > 22.0 && winter_declination < -22.0,
      "sinusoidal declination follows the annual seasonal cycle");
}

/* Vérifie que la déclinaison de Spencer est proche de zéro à l'équinoxe de printemps. */
static TestResult test_spencer_declination_is_near_spring_equinox(void) {
  /* Given */
  const int year = 2025;
  const double maximum_absolute_declination = 1.0;

  /* When */
  double declination = calculate_spencer_declination(
      (SolarDate){.jour_ordinal = 80, .annee = year});

  /* Then */
  return assert_condition(fabs(declination) < maximum_absolute_declination,
                          "Spencer declination is near zero at spring equinox");
}

/* Vérifie que la déclinaison de Spencer est positive au solstice d'été. */
static TestResult
test_spencer_declination_is_positive_at_summer_solstice(void) {
  /* Given */
  const int year = 2025;
  const double minimum_declination = 22.0;

  /* When */
  double declination = calculate_spencer_declination(
      (SolarDate){.jour_ordinal = 172, .annee = year});

  /* Then */
  return assert_condition(declination > minimum_declination,
                          "Spencer declination is positive at summer solstice");
}

/* Vérifie que la déclinaison de Spencer est négative au solstice d'hiver. */
static TestResult
test_spencer_declination_is_negative_at_winter_solstice(void) {
  /* Given */
  const int year = 2025;
  const double maximum_declination = -22.0;

  /* When */
  double declination = calculate_spencer_declination(
      (SolarDate){.jour_ordinal = 355, .annee = year});

  /* Then */
  return assert_condition(declination < maximum_declination,
                          "Spencer declination is negative at winter solstice");
}

/* Vérifie qu'un lieu situé à l'équateur reçoit douze heures de lumière du jour. */
static TestResult test_day_duration_is_twelve_hours_at_equator(void) {
  /* Given */
  const double declination = 23.45;
  const double latitude = 0.0;
  const double expected_duration = 12.0;

  /* When */
  double duration = calculate_day_duration((SolarObservation){
      .declinaison_degres = declination, .latitude_degres = latitude});

  /* Then */
  return assert_condition(fabs(duration - expected_duration) < 1e-9,
                          "equatorial day duration is twelve hours");
}

/* Vérifie qu'un seuil d'altitude solaire négatif allonge la durée du jour. */
static TestResult test_negative_solar_altitude_threshold_lengthens_day(void) {
  /* Given */
  const double declination = 0.0;
  const double latitude = 0.0;
  const double threshold = -0.833;
  const double pi = acos(-1.0);
  const double expected_duration =
      24.0 / pi * acos(sin(threshold * pi / 180.0));

  /* When */
  double duration = calculate_day_duration((SolarObservation){
      .declinaison_degres = declination,
      .latitude_degres = latitude,
      .altitude_solaire_seuil_degres = threshold});

  /* Then */
  return assert_condition(duration > 12.0 &&
                              fabs(duration - expected_duration) < 1e-9,
                          "negative solar threshold lengthens daylight");
}

/* Vérifie que le jour polaire est limité à 24 heures. */
static TestResult test_day_duration_clamps_to_polar_day(void) {
  /* Given */
  const double summer_declination = 23.45;
  const double high_northern_latitude = 80.0;
  const double expected_duration = 24.0;

  /* When */
  double duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = summer_declination,
                         .latitude_degres = high_northern_latitude});

  /* Then */
  return assert_condition(fabs(duration - expected_duration) < 1e-9,
                          "polar day duration is clamped to twenty-four hours");
}

/* Vérifie que la nuit polaire est limitée à zéro heure de lumière. */
static TestResult test_day_duration_clamps_to_polar_night(void) {
  /* Given */
  const double winter_declination = -23.45;
  const double high_northern_latitude = 80.0;
  const double expected_duration = 0.0;

  /* When */
  double duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = winter_declination,
                         .latitude_degres = high_northern_latitude});

  /* Then */
  return assert_condition(fabs(duration - expected_duration) < 1e-9,
                          "polar night duration is clamped to zero hours");
}

/* Vérifie les durées aux pôles, où l'altitude solaire reste constante. */
static TestResult test_day_duration_handles_exact_poles(void) {
  /* Given */
  const double threshold = 0.0;
  const double expected_day = 24.0;
  const double expected_night = 0.0;

  /* When */
  double north_polar_day = calculate_day_duration(
      (SolarObservation){.declinaison_degres = 23.45,
                         .latitude_degres = 90.0,
                         .altitude_solaire_seuil_degres = threshold});
  double south_polar_night = calculate_day_duration(
      (SolarObservation){.declinaison_degres = 23.45,
                         .latitude_degres = -90.0,
                         .altitude_solaire_seuil_degres = threshold});
  double at_threshold = calculate_day_duration(
      (SolarObservation){.declinaison_degres = threshold,
                         .latitude_degres = 90.0,
                         .altitude_solaire_seuil_degres = threshold});

  /* Then */
  return assert_condition(north_polar_day == expected_day &&
                              south_polar_night == expected_night &&
                              at_threshold == expected_night,
                          "exact-pole durations respect solar altitude threshold");
}

/* Vérifie le calcul aux latitudes proches des pôles et la limite des durées. */
static TestResult test_day_duration_handles_latitudes_near_poles(void) {
  /* Given */
  const double near_pole_latitude = 89.999;
  const double equinox_declination = 0.0;
  const double polar_day_declination = 1.0;
  const double threshold = 0.0;
  const double expected_equinox_duration = 12.0;
  const double expected_polar_day_duration = 24.0;
  const double expected_polar_night_duration = 0.0;

  /* When */
  double north_equinox_duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = equinox_declination,
                         .latitude_degres = near_pole_latitude,
                         .altitude_solaire_seuil_degres = threshold});
  double south_equinox_duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = equinox_declination,
                         .latitude_degres = -near_pole_latitude,
                         .altitude_solaire_seuil_degres = threshold});
  double northern_polar_day_duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = polar_day_declination,
                         .latitude_degres = near_pole_latitude,
                         .altitude_solaire_seuil_degres = threshold});
  double southern_polar_night_duration = calculate_day_duration(
      (SolarObservation){.declinaison_degres = polar_day_declination,
                         .latitude_degres = -near_pole_latitude,
                         .altitude_solaire_seuil_degres = threshold});

  /* Then */
  return assert_condition(
      fabs(north_equinox_duration - expected_equinox_duration) < 1e-9 &&
          fabs(south_equinox_duration - expected_equinox_duration) < 1e-9 &&
          northern_polar_day_duration == expected_polar_day_duration &&
          southern_polar_night_duration == expected_polar_night_duration,
      "near-pole durations remain bounded and follow the solar threshold");
}

/* Vérifie que la déclinaison de Meeus est proche de zéro à l'équinoxe de printemps. */
static TestResult test_meeus_declination_is_near_spring_equinox(void) {
  /* Given */
  const int year = 2026;
  const int spring_equinox_ordinal = 80;
  const double maximum_absolute_declination = 1.0;

  /* When */
  double declination = calculate_meeus_declination(
      (SolarDate){.jour_ordinal = spring_equinox_ordinal, .annee = year});

  /* Then */
  return assert_condition(fabs(declination) < maximum_absolute_declination,
                          "Meeus declination is near zero at spring equinox");
}

/* Compare la déclinaison de Meeus à la valeur de référence d'octobre 1992. */
static TestResult test_meeus_declination_matches_reference_date(void) {
  /* Given */
  const SolarDate reference_date = {.jour_ordinal = 287, .annee = 1992};
  const double expected_declination_at_noon = -7.97;
  const double tolerance = 0.02;

  /* When */
  double declination = calculate_meeus_declination(reference_date);

  /* Then */
  return assert_condition(
      fabs(declination - expected_declination_at_noon) < tolerance,
      "Meeus declination matches the 1992-10-13 noon reference value");
}

/* Compare les trois modèles aux points de référence saisonniers du JPL. */
static TestResult test_declination_models_match_jpl_seasonal_reference_grid(
    void) {
    /* Given */
    typedef struct {
        CalendarDate date;
        double expected_declination;
    } ReferencePoint;
    const ReferencePoint reference_points[] = {
            {{20, 3, 1900}, -0.224666230}, {{21, 6, 1900}, 23.450828147},
            {{22, 9, 1900}, 0.395171161},  {{21, 12, 1900}, -23.448320036},
            {{20, 3, 2000}, 0.072684024},  {{21, 6, 2000}, 23.437213351},
            {{22, 9, 2000}, 0.088595616},  {{21, 12, 2000}, -23.438129113},
            {{20, 3, 2100}, -0.017928265}, {{21, 6, 2100}, 23.427924581},
            {{22, 9, 2100}, 0.162720466},  {{21, 12, 2100}, -23.427724201},
            {{20, 3, 2500}, -0.005519173}, {{21, 6, 2500}, 23.370284382},
            {{22, 9, 2500}, 0.075975087},  {{21, 12, 2500}, -23.370924628},
            {{20, 3, 3000}, -0.089970364}, {{21, 6, 3000}, 23.305219200},
            {{22, 9, 3000}, 0.044863217},  {{21, 12, 3000}, -23.305014396},
    };
    const double sinusoidal_error_limit = 2.0;
    const double spencer_error_limit = 0.7;
    const double meeus_error_limit = 0.05;
    double maximum_sinusoidal_error = 0.0;
    double maximum_spencer_error = 0.0;
    double maximum_meeus_error = 0.0;

    /* When */
    for (size_t index = 0;
             index < sizeof(reference_points) / sizeof(reference_points[0]);
             index++) {
        const ReferencePoint *point = &reference_points[index];
        SolarDate solar_date = {.jour_ordinal = date_to_ordinal(point->date),
                                                        .annee = point->date.annee};
        double sinusoidal_error = fabs(
            calculate_sinusoidal_declination(solar_date) -
            point->expected_declination);
        double spencer_error = fabs(calculate_spencer_declination(solar_date) -
                                    point->expected_declination);
        double meeus_error = fabs(calculate_meeus_declination(solar_date) -
                                  point->expected_declination);
        if (sinusoidal_error > maximum_sinusoidal_error) {
            maximum_sinusoidal_error = sinusoidal_error;
        }
        if (spencer_error > maximum_spencer_error) {
            maximum_spencer_error = spencer_error;
        }
        if (meeus_error > maximum_meeus_error) {
            maximum_meeus_error = meeus_error;
        }
    }

    /* Then */
    char diagnostic[256];
    snprintf(diagnostic, sizeof(diagnostic),
             "maximum JPL errors: sinusoidal %.4f/%.2f, Spencer %.4f/%.2f, "
             "Meeus %.4f/%.2f degrees",
             maximum_sinusoidal_error, sinusoidal_error_limit,
             maximum_spencer_error, spencer_error_limit, maximum_meeus_error,
             meeus_error_limit);
    return assert_condition(
        maximum_sinusoidal_error < sinusoidal_error_limit &&
            maximum_spencer_error < spencer_error_limit &&
            maximum_meeus_error < meeus_error_limit,
        diagnostic);
}

/* Vérifie que la déclinaison de Meeus est proche de son maximum estival. */
static TestResult test_meeus_declination_is_near_summer_solstice_maximum(void) {
  /* Given */
  const int year = 2026;
  const int summer_solstice_ordinal = 172;
  const double minimum_declination = 22.0;
  const double maximum_declination = 24.5;

  /* When */
  double declination = calculate_meeus_declination(
      (SolarDate){.jour_ordinal = summer_solstice_ordinal, .annee = year});

  /* Then */
  return assert_condition(declination > minimum_declination &&
                              declination < maximum_declination,
                          "Meeus declination is near summer solstice maximum");
}

/* Vérifie que la déclinaison de Meeus est proche de son minimum hivernal. */
static TestResult test_meeus_declination_is_near_winter_solstice_minimum(void) {
  /* Given */
  const int year = 2026;
  const int winter_solstice_ordinal = 355;
  const double minimum_declination = -24.5;
  const double maximum_declination = -22.0;

  /* When */
  double declination = calculate_meeus_declination(
      (SolarDate){.jour_ordinal = winter_solstice_ordinal, .annee = year});

  /* Then */
  return assert_condition(declination > minimum_declination &&
                              declination < maximum_declination,
                          "Meeus declination is near winter solstice minimum");
}

static const TestCase test_cases[] = {
    {"test_sinusoidal_declination_tracks_seasonal_cycle",
     test_sinusoidal_declination_tracks_seasonal_cycle},
    {"test_spencer_declination_is_near_spring_equinox",
     test_spencer_declination_is_near_spring_equinox},
    {"test_spencer_declination_is_positive_at_summer_solstice",
     test_spencer_declination_is_positive_at_summer_solstice},
    {"test_spencer_declination_is_negative_at_winter_solstice",
     test_spencer_declination_is_negative_at_winter_solstice},
    {"test_day_duration_is_twelve_hours_at_equator",
     test_day_duration_is_twelve_hours_at_equator},
    {"test_negative_solar_altitude_threshold_lengthens_day",
     test_negative_solar_altitude_threshold_lengthens_day},
    {"test_day_duration_clamps_to_polar_day",
     test_day_duration_clamps_to_polar_day},
    {"test_day_duration_clamps_to_polar_night",
     test_day_duration_clamps_to_polar_night},
    {"test_day_duration_handles_exact_poles",
     test_day_duration_handles_exact_poles},
    {"test_day_duration_handles_latitudes_near_poles",
     test_day_duration_handles_latitudes_near_poles},
    {"test_meeus_declination_is_near_spring_equinox",
     test_meeus_declination_is_near_spring_equinox},
    {"test_meeus_declination_matches_reference_date",
     test_meeus_declination_matches_reference_date},
    {"test_declination_models_match_jpl_seasonal_reference_grid",
     test_declination_models_match_jpl_seasonal_reference_grid},
    {"test_meeus_declination_is_near_summer_solstice_maximum",
     test_meeus_declination_is_near_summer_solstice_maximum},
    {"test_meeus_declination_is_near_winter_solstice_minimum",
     test_meeus_declination_is_near_winter_solstice_minimum},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("solar", test_cases,
                        sizeof(test_cases) / sizeof(test_cases[0]));
}