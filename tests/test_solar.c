#include <math.h>
#include <stdio.h>

#include "date.h"
#include "solar.h"
#include "test_assertions.h"
#include "test_runner.h"
#include "web_api.h"

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

/* Vérifie l'altitude solaire maximale à midi solaire à l'équateur. */
static TestResult test_solar_altitude_reaches_zenith_at_equatorial_noon(void) {
  /* Given */
  const double expected_altitude = 90.0;

  /* When */
  double altitude = calculate_solar_altitude(12.0, 0.0, 0.0);

  /* Then */
  return assert_condition(fabs(altitude - expected_altitude) < 1e-9,
                          "solar altitude reaches zenith at equatorial noon");
}

/* Vérifie que le fuseau décale ensemble le lever et le coucher. */
static TestResult test_solar_events_follow_timezone_offset(void) {
  /* Given */
  const CalendarDate date = {.jour = 20, .mois = 3, .annee = 2024};
  const double latitude = 0.0;
  const double longitude = 0.0;
  const double declination = 0.0;
  const double threshold = 0.0;
  const double timezone_offset_minutes = 120.0;

  /* When */
  double sunrise_utc = calculate_solar_event_hour(
      date, latitude, longitude, declination, threshold, 0.0, 1);
  double sunrise_local = calculate_solar_event_hour(
      date, latitude, longitude, declination, threshold,
      timezone_offset_minutes, 1);
  double sunset_utc = calculate_solar_event_hour(
      date, latitude, longitude, declination, threshold, 0.0, 0);
  double sunset_local = calculate_solar_event_hour(
      date, latitude, longitude, declination, threshold,
      timezone_offset_minutes, 0);

  /* Then */
  return assert_condition(fabs(sunrise_local - sunrise_utc - 2.0) < 1e-9 &&
                              fabs(sunset_local - sunset_utc - 2.0) < 1e-9,
                          "solar events shift with the civil timezone offset");
}

/* Vérifie qu'aucun horaire artificiel n'est produit sans franchissement du seuil. */
static TestResult test_solar_events_are_nan_when_threshold_is_never_crossed(void) {
  /* Given */
  const CalendarDate summer_date = {.jour = 21, .mois = 6, .annee = 2025};
  const CalendarDate winter_date = {.jour = 21, .mois = 12, .annee = 2025};
  const double latitude = 80.0;
  const double threshold = -0.833;

  /* When */
  double polar_summer_sunrise = calculate_solar_event_hour(
      summer_date, latitude, 0.0, 23.45, threshold, 0.0, 1);
  double polar_summer_sunset = calculate_solar_event_hour(
      summer_date, latitude, 0.0, 23.45, threshold, 0.0, 0);
  double polar_winter_sunrise = calculate_solar_event_hour(
      winter_date, latitude, 0.0, -23.45, threshold, 0.0, 1);
  double polar_winter_sunset = calculate_solar_event_hour(
      winter_date, latitude, 0.0, -23.45, threshold, 0.0, 0);

  /* Then */
  return assert_condition(isnan(polar_summer_sunrise) &&
                              isnan(polar_summer_sunset) &&
                              isnan(polar_winter_sunrise) &&
                              isnan(polar_winter_sunset),
                          "solar event times are undefined without threshold crossings");
}

/* Vérifie que l'API web sélectionne les modèles C et rejette les entrées invalides. */
static TestResult test_web_api_declination_selects_models_and_rejects_invalid_input(void) {
  /* Given */
  const CalendarDate date = {.jour = 20, .mois = 3, .annee = 2024};
  const SolarDate solar_date = {
      .jour_ordinal = date_to_ordinal(date), .annee = date.annee};

  /* When */
  double sinusoidal = solar_web_declination(1, 2024, 3, 20);
  double spencer = solar_web_declination(2, 2024, 3, 20);
  double meeus = solar_web_declination(3, 2024, 3, 20);
  double invalid_model = solar_web_declination(99, 2024, 3, 20);
  double invalid_date = solar_web_declination(3, 2024, 2, 30);

  /* Then */
  return assert_condition(
      sinusoidal == calculate_sinusoidal_declination(solar_date) &&
          spencer == calculate_spencer_declination(solar_date) &&
          meeus == calculate_meeus_declination(solar_date) &&
          isnan(invalid_model) && isnan(invalid_date),
      "web API selects each C model and rejects invalid inputs");
}

/* Vérifie la durée exposée par l'API web et ses validations de latitude/seuil. */
static TestResult test_web_api_day_duration_validates_observation(void) {
  /* Given */
  const int model = 3;
  const int year = 2024;
  const int month = 3;
  const int day = 20;
  const double declination = solar_web_declination(model, year, month, day);
  const double expected_duration = calculate_day_duration((SolarObservation){
      .declinaison_degres = declination,
      .latitude_degres = 48.0,
      .altitude_solaire_seuil_degres = -0.833});

  /* When */
  double duration =
      solar_web_day_duration(model, year, month, day, 48.0, -0.833);
  double invalid_latitude =
      solar_web_day_duration(model, year, month, day, 91.0, -0.833);
  double invalid_threshold =
      solar_web_day_duration(model, year, month, day, 48.0, -91.0);

  /* Then */
  return assert_condition(duration == expected_duration &&
                              isnan(invalid_latitude) &&
                              isnan(invalid_threshold),
                          "web API validates the daylight observation");
}

/* Vérifie que l'API web expose l'altitude et l'heure d'événement du noyau C. */
static TestResult test_web_api_exposes_altitude_and_solar_event(void) {
  /* Given */
  const CalendarDate date = {.jour = 20, .mois = 3, .annee = 2024};
  const double latitude = 48.0;
  const double longitude = 2.0;
  const double declination = 0.0;
  const double threshold = -0.833;
  const double timezone_offset = 60.0;

  /* When */
  double altitude = solar_web_altitude(12.0, latitude, declination);
  double sunrise = solar_web_event_hour(
      date.annee, date.mois, date.jour, latitude, longitude, declination,
      threshold, timezone_offset, 1);
  double expected_sunrise = calculate_solar_event_hour(
      date, latitude, longitude, declination, threshold, timezone_offset, 1);
  double invalid_event = solar_web_event_hour(
      2024, 2, 30, latitude, longitude, declination, threshold,
      timezone_offset, 1);

  /* Then */
  return assert_condition(
      altitude == calculate_solar_altitude(12.0, latitude, declination) &&
          sunrise == expected_sunrise && isnan(invalid_event),
      "web API exposes C altitude and validates solar event dates");
}

/* Vérifie les bornes numériques des fonctions altitude et événement de l'API web. */
static TestResult test_web_api_rejects_invalid_altitude_and_event_inputs(void) {
  /* Given */
  const int year = 2024;
  const int month = 3;
  const int day = 20;
  const double valid_latitude = 48.0;
  const double valid_longitude = 2.0;
  const double valid_declination = 0.0;
  const double valid_threshold = -0.833;
  const double valid_timezone = 60.0;

  /* When */
  double invalid_altitude_hour =
      solar_web_altitude(25.0, valid_latitude, valid_declination);
  double invalid_altitude_latitude =
      solar_web_altitude(12.0, 91.0, valid_declination);
  double invalid_altitude_declination =
      solar_web_altitude(12.0, valid_latitude, NAN);
  double invalid_event_year = solar_web_event_hour(
      1800, month, day, valid_latitude, valid_longitude, valid_declination,
      valid_threshold, valid_timezone, 1);
  double invalid_event_latitude = solar_web_event_hour(
      year, month, day, 91.0, valid_longitude, valid_declination,
      valid_threshold, valid_timezone, 1);
  double invalid_event_longitude = solar_web_event_hour(
      year, month, day, valid_latitude, 181.0, valid_declination,
      valid_threshold, valid_timezone, 1);
  double invalid_event_declination = solar_web_event_hour(
      year, month, day, valid_latitude, valid_longitude, 91.0,
      valid_threshold, valid_timezone, 1);
  double invalid_event_threshold = solar_web_event_hour(
      year, month, day, valid_latitude, valid_longitude, valid_declination,
      91.0, valid_timezone, 1);
  double invalid_event_timezone = solar_web_event_hour(
      year, month, day, valid_latitude, valid_longitude, valid_declination,
      valid_threshold, NAN, 1);
  double invalid_event_kind = solar_web_event_hour(
      year, month, day, valid_latitude, valid_longitude, valid_declination,
      valid_threshold, valid_timezone, 2);

  /* Then */
  return assert_condition(
      isnan(invalid_altitude_hour) && isnan(invalid_altitude_latitude) &&
          isnan(invalid_altitude_declination) && isnan(invalid_event_year) &&
          isnan(invalid_event_latitude) && isnan(invalid_event_longitude) &&
          isnan(invalid_event_declination) && isnan(invalid_event_threshold) &&
          isnan(invalid_event_timezone) && isnan(invalid_event_kind),
      "web API rejects non-finite and out-of-range altitude/event inputs");
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
    {"test_solar_altitude_reaches_zenith_at_equatorial_noon",
     test_solar_altitude_reaches_zenith_at_equatorial_noon},
    {"test_solar_events_follow_timezone_offset",
     test_solar_events_follow_timezone_offset},
    {"test_solar_events_are_nan_when_threshold_is_never_crossed",
     test_solar_events_are_nan_when_threshold_is_never_crossed},
    {"test_web_api_declination_selects_models_and_rejects_invalid_input",
     test_web_api_declination_selects_models_and_rejects_invalid_input},
    {"test_web_api_day_duration_validates_observation",
     test_web_api_day_duration_validates_observation},
    {"test_web_api_exposes_altitude_and_solar_event",
     test_web_api_exposes_altitude_and_solar_event},
    {"test_web_api_rejects_invalid_altitude_and_event_inputs",
     test_web_api_rejects_invalid_altitude_and_event_inputs},
};

/* Exécuteur */

int main(void) {
  return run_test_suite("solar", test_cases,
                        sizeof(test_cases) / sizeof(test_cases[0]));
}