// --- Fichier : solar.h ---

#ifndef SOLAR_H
#define SOLAR_H

#include "date.h"

typedef struct {
	int jour_ordinal;
	int annee;
} SolarDate;

typedef struct {
	double declinaison_degres;
	double latitude_degres;
	double altitude_solaire_seuil_degres;
} SolarObservation;

typedef double (*DeclinationFunc)(SolarDate date);

double calculate_day_duration(SolarObservation observation);

double calculate_sinusoidal_declination(SolarDate date);
double calculate_spencer_declination(SolarDate date);
double calculate_meeus_declination(SolarDate date);
double calculate_solar_altitude(double hour, double latitude_degrees,
								double declination_degrees);
double calculate_solar_event_hour(CalendarDate date, double latitude_degrees,
								  double longitude_degrees,
								  double declination_degrees,
								  double altitude_threshold_degrees,
								  double timezone_offset_minutes,
								  int is_sunrise);

#endif // SOLAR_H
