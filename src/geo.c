// --- Fichier : geo.c ---

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config.h"
#include "geo.h"
#include "logger.h"

// ==========================================================================
// FONCTIONS UTILITAIRES PRIVÉES DE GEO.C
// ==========================================================================

typedef struct {
  char direction;
  char coordinates[MAX_BUF_SIZE];
} ParsedCoordinates;

/**
 * @brief Extrait la direction (N/S) et la partie numérique des coordonnées
 * géographiques.
 *
 * Cette fonction isole la logique de découpage de la chaîne et de vérification
 * de la direction.
 *
 * @param ligne La chaîne complète des coordonnées (ex: "47 deg 17 min 48 sec
 * Nord").
 * @param direction Pointeur où stocker 'N' ou 'S'.
 * @param coord_part Tampon de sortie pour stocker la partie numérique brute.
 * @return true si la direction est valide et la chaîne est correctement
 * découpée, false sinon.
 */
static bool extract_direction_and_coords(const char *ligne,
                                         ParsedCoordinates *coordinates) {
  const char *last_space = strrchr(ligne, ' ');
  if (!last_space) {
    log_error("Impossible de trouver l'espace séparateur de direction dans la "
              "ligne: '%s'",
              ligne);
    return false;
  }

  // --- 1. Identification de la direction avec strcasecmp pour la robustesse ---
  const char *direction_str = last_space + 1;

  if (strcasecmp(direction_str, "Nord") == 0) {
    coordinates->direction = 'N';
  } else if (strcasecmp(direction_str, "Sud") == 0) {
    coordinates->direction = 'S';
  } else {
    log_error("Direction invalide trouvée à la fin de la ligne: '%s'",
              direction_str);
    return false;
  }

  // --- 2. Tronquer la chaîne pour garder uniquement les coordonnées ---
  size_t len_coords = last_space - ligne;
  if (len_coords >= MAX_BUF_SIZE)
    len_coords = MAX_BUF_SIZE - 1;

  memcpy(coordinates->coordinates, ligne, len_coords);
  coordinates->coordinates[len_coords] = '\0';

  return true;
}

static const char *skip_whitespace(const char *text) {
  while (isspace((unsigned char)*text)) {
    text++;
  }
  return text;
}

static bool parse_integer_component(const char **cursor, const char *unit,
                                    int *value) {
  const char *number_start = skip_whitespace(*cursor);
  errno = 0;
  char *number_end;
  long parsed_value = strtol(number_start, &number_end, 10);
  if (errno == ERANGE || number_end == number_start ||
      parsed_value < INT_MIN || parsed_value > INT_MAX ||
      !isspace((unsigned char)*number_end)) {
    return false;
  }

  const char *unit_start = skip_whitespace(number_end);
  size_t unit_length = strlen(unit);
  if (strlen(unit_start) <= unit_length ||
      strncmp(unit_start, unit, unit_length) != 0 ||
      !isspace((unsigned char)unit_start[unit_length])) {
    return false;
  }

  *value = (int)parsed_value;
  *cursor = unit_start + unit_length;
  return true;
}

static bool parse_seconds_component(const char **cursor, double *seconds) {
  const char *number_start = skip_whitespace(*cursor);
  errno = 0;
  char *number_end;
  double parsed_seconds = strtod(number_start, &number_end);
  if (errno == ERANGE || number_end == number_start ||
      !isspace((unsigned char)*number_end)) {
    return false;
  }

  const char *unit_start = skip_whitespace(number_end);
  if (strncmp(unit_start, "sec", 3) != 0 ||
      (unit_start[3] != '\0' && !isspace((unsigned char)unit_start[3])) ||
      *skip_whitespace(unit_start + 3) != '\0') {
    return false;
  }

  *seconds = parsed_seconds;
  return true;
}

static bool parse_latitude_components(const char *coordinates, int *degrees,
                                      int *minutes, double *seconds) {
  const char *cursor = coordinates;
  return parse_integer_component(&cursor, "deg", degrees) &&
         parse_integer_component(&cursor, "min", minutes) &&
         parse_seconds_component(&cursor, seconds);
}

// ==========================================================================
// FONCTIONS PUBLIQUES
// ==========================================================================

/**
 * @brief Convertit une chaîne de coordonnées géographiques (ex: "47 deg 17 min
 * 48 sec Nord") en une valeur de latitude en double.
 *
 * REMARQUE : Cette fonction se concentre uniquement sur l'analyse syntaxique.
 * La validation des bornes globales (-90 à 90) doit être effectuée par
 * l'appelant (main.c).
 *
 * @param ligne La chaîne contenant les coordonnées.
 * @param result Pointeur où stocker la latitude en double.
 * @return int ERR_OK si le parsing est syntaxiquement réussi, ERR_CONVERSION
 * sinon.
 */
int parse_latitude_string(const char *ligne, double *result) {
  int degrees = 0, minutes = 0;
  double seconds = 0.0;
  ParsedCoordinates coordinates;

  if (!ligne || !result) {
    log_error("Tentative de parsing avec ligne ou résultat NULL.");
    return ERR_CONVERSION;
  }

  // --- 1. Extraction (Responsabilité isolée) ---
  if (!extract_direction_and_coords(ligne, &coordinates)) {
    return ERR_CONVERSION; // Échec de l'extraction
  }

  // --- 2. Parsing des composantes numériques ---
  if (!parse_latitude_components(coordinates.coordinates, &degrees, &minutes,
                                 &seconds)) {
    log_debug("Format de coordonnées invalide dans '%s'.",
              coordinates.coordinates);
    return ERR_CONVERSION;
  }

  // --- 3. Vérifications INTERNES des composantes (Logique de formatage) ---
  if (degrees < 0 || degrees > 90 || minutes < 0 || minutes >= 60 ||
      !isfinite(seconds) || seconds < 0 || seconds >= 60) {
    log_error("Composantes de coordonnées hors plage de format (Deg: %d, Min: "
              "%d, Sec: %.2lf) trouvées dans '%s'.",
              degrees, minutes, seconds, coordinates.coordinates);
    return ERR_CONVERSION; // Échec de validation interne du format
  }

  // --- 4. Conversion Finale ---
  double lat =
      (double)degrees + (double)minutes / 60.0 + (double)seconds / 3600.0;
  if (coordinates.direction == 'S')
    lat = -lat;

  *result = lat;
  log_info("Latitude parsée avec succès: %f", lat);
  return ERR_OK; // Succès de la transformation syntaxique
}

/**
 * @brief Convertit une latitude en double en format Degrés° Minutes' Secondes".
 * @param lat La latitude en double.
 * @param buf Le tampon de sortie.
 * @param size La taille maximale du buffer.
 */
void format_latitude(double lat, char *buf, size_t size) {
  char hemisphere = (lat >= 0) ? 'N' : 'S';
  double abs_lat = fabs(lat);
  int deg = (int)abs_lat;
  double frac = abs_lat - deg;

  double total_minutes = frac * 60.0;
  int min = (int)total_minutes;
  double sec_frac = total_minutes - min;
  int sec = (int)round(sec_frac * 60.0);

  // Gestion des débordements (sec -> min -> deg)
  if (sec == 60) {
    sec = 0;
    min += 1;
  }
  if (min == 60) {
    min = 0;
    deg += 1;
  }

  snprintf(buf, size, "%d° %d′ %d″ %c", deg, min, sec, hemisphere);
}
