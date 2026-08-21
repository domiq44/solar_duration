// --- File: string_utils.c ---

#include <ctype.h>
#include <string.h>

#include "string_utils.h"

/**
 * @brief Supprime les espaces blancs au début et à la fin d'une chaîne.
 * @param str La chaîne à nettoyer (doit être modifiable).
 * @return char* Le pointeur vers la chaîne nettoyée.
 */
char *strtrim(char *str) {
  // Nettoyage du début
  while (isspace((unsigned char)*str))
    str++;

  if (*str == '\0')
    return str;

  // Nettoyage de la fin
  char *end = str + strlen(str) - 1;
  while (end >= str && isspace((unsigned char)*end))
    end--;
  *(end + 1) = '\0';
  return str;
}
