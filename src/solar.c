// --- Fichier : solar.c ---

#include <math.h>
#include <stdio.h>

#include "config.h"
#include "date.h"
#include "solar.h"

// Définition de M_PI si elle n'est pas définie par math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ==========================================================================
// STRUCTURES ET CONSTANTES GLOBALES
// ==========================================================================

/**
 * @brief Constante representant l'inclinaison moyenne de l'ecliptique de la
 * Terre (Obliquite).
 * @details Cette valeur est le maximum theorique de la declinaison solaire.
 * @reference Astronomie classique (Obliquite de l'ecliptique).
 */
#define MEAN_SOLAR_DECLINATION_DEG 23.45

/**
 * @brief Constante d'ajustement de phase pour cibler l'equinoce de printemps.
 * @details Pour que $\delta = 0$ aux equinoxes, nous devons centrer le cycle
 * sinusoidal. Nous utilisons le Jour Ordinal 81 comme point de reference pour
 * le printemps.
 * @note Cette constante est le point de calibration le plus sensible.
 */
#define EQUINOX_SPRING_DAY_ORDINAL 81.0

// ==========================================================================
// FONCTION DE DECLINAISON SOLAIRE (MODELE SINUSOIDAL CALIBRE)
// ==========================================================================

/**
 * @brief Calcule la declinaison solaire ($\delta$) en degres.
 *
 * Ce modele utilise une approximation sinusoidale de l'orbite terrestre,
 * calibree pour que $\delta = 0^\circ$ aux dates des equinoxes (Jour Ordinal
 * $\approx 81$ et $\approx 266$).
 *
 * @param jour_n Jour ordinal de l'annee (1 à 365 ou 366).
 * @param annee L'annee (necessaire pour determiner si elle est bissextile).
 * @return double La declinaison solaire en degres (variant autour de
 * $\pm 23.45^\circ$).
 * @reference Modelisation cyclique de la position solaire annuelle.
 */
double calculate_sinusoidal_declination(SolarDate date) {
  // 1. Determination de la longueur de l'annee
  int jours_total = is_leap_year(date.annee) ? 366 : 365;

  // 2. Calcul de la fraction de l'annee ecoulee (0.0 à 1.0)
  double fraction_annee = (double)date.jour_ordinal / jours_total;

  // 3. Calcul de la phase ajustee (Le coeur de la calibration)
  // En soustrayant l'offset de l'equinoxe, on force $\sin(\text{phase})=0$ à ce
  // jour.
  double phase_ajustee =
      (fraction_annee - (EQUINOX_SPRING_DAY_ORDINAL / jours_total)) * 2.0 *
      M_PI;

  // 4. Calcul de la declinaison
  // L'amplitude est definie par l'inclinaison moyenne.
  double delta = MEAN_SOLAR_DECLINATION_DEG * sin(phase_ajustee);

  return delta; // Resultat en degres.
}

// ==========================================================================
// FONCTION DE DECLINAISON SOLAIRE (MODELE SPENCER CALIBRE)
// ==========================================================================

double calculate_spencer_declination(SolarDate date) {
  double phase_ajustee =
      ((double)date.jour_ordinal - 1.0) * 2.0 * M_PI / 365.0;

  // 4. Calcul de la declinaison
  double delta =
      0.006918 - 0.399912 * cos(phase_ajustee) + 0.070257 * sin(phase_ajustee) -
      0.006758 * cos(2 * phase_ajustee) + 0.000907 * sin(2 * phase_ajustee) -
      0.002697 * cos(3 * phase_ajustee) + 0.001480 * sin(3 * phase_ajustee);

  return delta * 180 / M_PI;
}

// ==========================================================================
// FONCTION DE DECLINAISON SOLAIRE (MODELE MEEUS CALIBRE)
// ==========================================================================

static double julian_day(CalendarDate date) {
  int adjusted_year = date.annee;
  int adjusted_month = date.mois;

  if (adjusted_month <= 2) {
    adjusted_year--;
    adjusted_month += 12;
  }

  int century = adjusted_year / 100;
  int calendar_correction = 2 - century + century / 4;

  return floor(365.25 * (adjusted_year + 4716)) +
         floor(30.6001 * (adjusted_month + 1)) + date.jour +
         calendar_correction - 1524.5 + 0.5;
}

/**
 * @brief Calcule la déclinaison solaire selon l'approximation Meeus.
 * @param jour_n Jour ordinal de l'année (1 à 365 ou 366).
 * @param annee Année civile grégorienne.
 * @return Déclinaison solaire en degrés.
 */
double calculate_meeus_declination(SolarDate date) {
  CalendarDate year_start = {.jour = 1, .mois = 1, .annee = date.annee};
  double julian_date = julian_day(year_start) + date.jour_ordinal - 1;
  double centuries = (julian_date - 2451545.0) / 36525.0;

  double mean_longitude = fmod(280.46646 + 36000.76983 * centuries +
                                   0.0003032 * centuries * centuries,
                               360.0);
  double mean_anomaly = fmod(357.52911 + 35999.05029 * centuries -
                                 0.0001537 * centuries * centuries,
                             360.0) *
                        M_PI / 180.0;

  double equation_of_center =
      (1.914602 - 0.004817 * centuries - 0.000014 * centuries * centuries) *
          sin(mean_anomaly) +
      (0.019993 - 0.000101 * centuries) * sin(2.0 * mean_anomaly) +
      0.000289 * sin(3.0 * mean_anomaly);

  double true_longitude = mean_longitude + equation_of_center;
  double omega = (125.04 - 1934.136 * centuries) * M_PI / 180.0;
  double apparent_longitude =
      (true_longitude - 0.00569 - 0.00478 * sin(omega)) * M_PI / 180.0;

  double mean_obliquity = 23.43929111 - 0.013004167 * centuries -
                          0.000000164 * centuries * centuries +
                          0.000000504 * centuries * centuries * centuries;
  double obliquity = (mean_obliquity + 0.00256 * cos(omega)) * M_PI / 180.0;

  return asin(sin(obliquity) * sin(apparent_longitude)) * 180.0 / M_PI;
}

static double calculate_equation_of_time_minutes(CalendarDate date) {
  double centuries = (julian_day(date) - 2451545.0) / 36525.0;
  double mean_longitude =
      (280.46646 + 36000.76983 * centuries +
       0.0003032 * centuries * centuries) *
      M_PI / 180.0;
  double mean_anomaly =
      (357.52911 + 35999.05029 * centuries -
       0.0001537 * centuries * centuries) *
      M_PI / 180.0;
  double eccentricity =
      0.016708634 - 0.000042037 * centuries - 0.0000001267 * centuries * centuries;
  double omega = (125.04 - 1934.136 * centuries) * M_PI / 180.0;
  double obliquity =
      (23.43929111 - 0.013004167 * centuries -
       0.000000164 * centuries * centuries +
       0.000000504 * centuries * centuries * centuries +
       0.00256 * cos(omega)) *
      M_PI / 180.0;
  double y = tan(obliquity / 2.0) * tan(obliquity / 2.0);
  double equation =
      y * sin(2.0 * mean_longitude) - 2.0 * eccentricity * sin(mean_anomaly) +
      4.0 * eccentricity * y * sin(mean_anomaly) * cos(2.0 * mean_longitude) -
      0.5 * y * y * sin(4.0 * mean_longitude) -
      1.25 * eccentricity * eccentricity * sin(2.0 * mean_anomaly);

  return equation * 180.0 / M_PI * 4.0;
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
double calculate_solar_altitude(double hour, double latitude_degrees,
                                double declination_degrees) {
  double latitude = latitude_degrees * M_PI / 180.0;
  double declination = declination_degrees * M_PI / 180.0;
  double hour_angle = (hour - 12.0) * 15.0 * M_PI / 180.0;
  double sine_altitude = sin(latitude) * sin(declination) +
                         cos(latitude) * cos(declination) * cos(hour_angle);

  if (sine_altitude > 1.0)
    sine_altitude = 1.0;
  if (sine_altitude < -1.0)
    sine_altitude = -1.0;

  return asin(sine_altitude) * 180.0 / M_PI;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
double calculate_solar_event_hour(CalendarDate date, double latitude_degrees,
                                  double longitude_degrees,
                                  double declination_degrees,
                                  double altitude_threshold_degrees,
                                  double timezone_offset_minutes,
                                  int is_sunrise) {
  if (!isfinite(latitude_degrees) || fabs(latitude_degrees) >= 90.0 ||
      !isfinite(longitude_degrees) || !isfinite(declination_degrees) ||
      !isfinite(altitude_threshold_degrees) ||
      !isfinite(timezone_offset_minutes)) {
    return NAN;
  }

  double latitude = latitude_degrees * M_PI / 180.0;
  double declination = declination_degrees * M_PI / 180.0;
  double threshold = altitude_threshold_degrees * M_PI / 180.0;
  double cos_hour_angle =
      (sin(threshold) - sin(latitude) * sin(declination)) /
      (cos(latitude) * cos(declination));

  if (!isfinite(cos_hour_angle) || cos_hour_angle > 1.0 + 1e-12 ||
      cos_hour_angle < -1.0 - 1e-12) {
    return NAN;
  }
  if (cos_hour_angle > 1.0)
    cos_hour_angle = 1.0;
  if (cos_hour_angle < -1.0)
    cos_hour_angle = -1.0;

  double hour_angle_degrees = acos(cos_hour_angle) * 180.0 / M_PI;
  double solar_noon_minutes =
      720.0 - calculate_equation_of_time_minutes(date) -
      4.0 * longitude_degrees + timezone_offset_minutes;
  double event_minutes =
      solar_noon_minutes + (is_sunrise ? -4.0 : 4.0) * hour_angle_degrees;

  return event_minutes / 60.0;
}
// NOLINTEND(bugprone-easily-swappable-parameters)

// ==========================================================================
// FONCTION DE CALCUL DE DUREE DU JOUR
// ==========================================================================

/**
 * @brief Calcule la duree du jour en heures (Jours de lumiere).
 *
 * Prend la declinaison en paramètre pour éviter des recalculs.
 *
 * @param delta_deg La declinaison solaire (en degres) déjà calculée.
 * @param latitude_deg La latitude du point d'observation en degres.
 * @return double La duree du jour en heures.
 */
double calculate_day_duration(SolarObservation observation) {

  // 1. Conversion des angles en radians
  double phi_rad = observation.latitude_degres * M_PI / 180.0;
  double delta_rad = observation.declinaison_degres * M_PI / 180.0;
  double altitude_seuil_rad =
      observation.altitude_solaire_seuil_degres * M_PI / 180.0;

  if (fabs(observation.latitude_degres) == 90.0) {
    double solar_altitude = observation.latitude_degres > 0.0
                                ? observation.declinaison_degres
                                : -observation.declinaison_degres;
    return solar_altitude > observation.altitude_solaire_seuil_degres ? 24.0
                                                                      : 0.0;
  }

  // 2. Calcul de l'angle horaire à partir de l'altitude solaire seuil.
  double cos_omega =
      (sin(altitude_seuil_rad) - sin(phi_rad) * sin(delta_rad)) /
      (cos(phi_rad) * cos(delta_rad));

  // 3. Securite critique (Clamping)
  if (cos_omega > 1.0)
    cos_omega = 1.0;
  if (cos_omega < -1.0)
    cos_omega = -1.0;

  double omega = acos(cos_omega); // omega est en radians

  // 4. Calcul de la duree en heures
  double duree = 2 * omega * (12.0 / M_PI);

  // Le message de débogage est désormais géré par main.c.
  return duree;
}
