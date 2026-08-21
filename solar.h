// --- File: solar.h ---

#ifndef SOLAR_H
#define SOLAR_H

// 1. Définir le type de la fonction de déclinaison
// Elle prend l'ordinal et l'année, et retourne la déclinaison en degrés.
typedef double (*DeclinationFunc)(int jour_n, int annee);

// 2. Les fonctions existantes restent pour l'implémentation
double calculate_declination(int jour_n, int annee, int mode);
double calculate_day_duration(double delta_deg, double latitude_deg);

// Exposer les fonctions statiques pour le binding
double calculate_sinusoidal_declination(int jour_n, int annee);
double calculate_spencer_declination(int jour_n, int annee);
double calculate_meeus_declination(int jour_n, int annee);

#endif // SOLAR_H
