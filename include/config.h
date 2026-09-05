// --- File: config.h ---

#ifndef CONFIG_H
#define CONFIG_H

// --- Constantes de Taille/Buffer ---
#define MAX_BUF_SIZE (256)

// --- Constantes de Validation des Données ---
#define MIN_YEAR (1900)
#define MAX_YEAR (3000)
#define MIN_LATITUDE (-90.0)
#define MAX_LATITUDE (90.0)
#define MIN_SOLSTICE_DAY (1)
#define MAX_SOLSTICE_DAY (366)

// --- Constantes de Mode de Calcul Solaire ---
#define MODE_SINUSOIDAL (1)
#define MODE_SPENCER (2)
#define MODE_MEEUS (3)

#endif // CONFIG_H
