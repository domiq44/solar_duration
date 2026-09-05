// --- File: geo.h ---

#ifndef GEO_H
#define GEO_H

#include <stddef.h>

#include "config.h"

// Codes d'erreur
#define ERR_OK (0)
// ERR_CONVERSION est utilisé pour signifier un échec de parsing syntaxique
#define ERR_CONVERSION (-1)

// La validation des composantes est effectuée pendant le parsing.
// La configuration finale vérifie ensuite les bornes globales de latitude.
int parse_latitude_string(const char *ligne, double *result);
void format_latitude(double lat, char *buf, size_t size);

#endif // GEO_H
