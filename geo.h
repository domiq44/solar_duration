// --- File: geo.h ---

#ifndef GEO_H
#define GEO_H

#include <stddef.h>

#include "config.h"

// Codes d'erreur
#define ERR_OK (0)
// ERR_CONVERSION est utilisé pour signifier un échec de parsing syntaxique
#define ERR_CONVERSION (-1)

// NOTE: La validation des bornes (-90 à 90) est gérée par main.c
int parse_latitude_string(const char *ligne, double *result);
void format_latitude(double lat, char *buf, size_t size);

#endif // GEO_H
