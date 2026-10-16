// --- Fichier : solar.h ---

#ifndef SOLAR_H
#define SOLAR_H

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

#endif // SOLAR_H
