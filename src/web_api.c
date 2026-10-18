#include <math.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define WEB_API_EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define WEB_API_EXPORT
#endif

#include "config.h"
#include "date.h"
#include "solar.h"
#include "web_api.h"

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
WEB_API_EXPORT double solar_web_declination(int model, int year, int month,
                                             int day) {
  CalendarDate calendar_date = {.jour = day, .mois = month, .annee = year};
  int ordinal = date_to_ordinal(calendar_date);
  if (ordinal < 1 || ordinal > 366 || year < MIN_YEAR || year > MAX_YEAR) {
    return NAN;
  }

  SolarDate solar_date = {.jour_ordinal = ordinal, .annee = year};
  switch (model) {
  case MODE_SINUSOIDAL:
    return calculate_sinusoidal_declination(solar_date);
  case MODE_SPENCER:
    return calculate_spencer_declination(solar_date);
  case MODE_MEEUS:
    return calculate_meeus_declination(solar_date);
  default:
    return NAN;
  }
}

WEB_API_EXPORT double solar_web_day_duration(int model, int year, int month,
                                              int day, double latitude_degrees,
                                              double altitude_threshold_degrees) {
  double declination = solar_web_declination(model, year, month, day);
  if (!isfinite(declination) || !isfinite(latitude_degrees) ||
      latitude_degrees < MIN_LATITUDE || latitude_degrees > MAX_LATITUDE ||
      !isfinite(altitude_threshold_degrees) ||
      altitude_threshold_degrees < MIN_SOLAR_ALTITUDE_DEGREES ||
      altitude_threshold_degrees > MAX_SOLAR_ALTITUDE_DEGREES) {
    return NAN;
  }

  return calculate_day_duration((SolarObservation){
      .declinaison_degres = declination,
      .latitude_degres = latitude_degrees,
      .altitude_solaire_seuil_degres = altitude_threshold_degrees});
}

WEB_API_EXPORT double solar_web_altitude(double hour, double latitude_degrees,
                                          double declination_degrees) {
  return calculate_solar_altitude(hour, latitude_degrees,
                                  declination_degrees);
}

WEB_API_EXPORT double solar_web_event_hour(
    int year, int month, int day, double latitude_degrees,
    double longitude_degrees, double declination_degrees,
    double altitude_threshold_degrees, double timezone_offset_minutes,
    int is_sunrise) {
  CalendarDate date = {.jour = day, .mois = month, .annee = year};
  if (!is_date_valid(date)) {
    return NAN;
  }

  return calculate_solar_event_hour(
      date, latitude_degrees, longitude_degrees, declination_degrees,
      altitude_threshold_degrees, timezone_offset_minutes, is_sunrise != 0);
}
// NOLINTEND(bugprone-easily-swappable-parameters)