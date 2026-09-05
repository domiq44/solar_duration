// --- File: string_utils.h ---

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

/**
 * @brief Supprime les espaces blancs au début et à la fin d'une chaîne.
 * @param str La chaîne à nettoyer (doit être modifiable).
 * @return char* Le pointeur vers la chaîne nettoyée.
 */
char *strtrim(char *str);

#endif // STRING_UTILS_H
