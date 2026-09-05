// --- File: geo.c ---

#include <ctype.h>
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
 * @param coord_part Buffer de sortie pour stocker la partie numérique brute.
 * @return true si la direction est valide et la chaîne est correctement
 * découpée, false sinon.
 */
static bool extract_direction_and_coords(const char *ligne, char *direction,
                                         char *coord_part) {
  const char *last_space = strrchr(ligne, ' ');
  if (!last_space) {
    log_error("Impossible de trouver l'espace séparateur de direction dans la "
              "ligne: '%s'",
              ligne);
    return false;
  }

  // --- 1. Identification de la Direction (Utilisation de strcasecmp pour la
  // robustesse) ---
  const char *direction_str = last_space + 1;

  if (strcasecmp(direction_str, "Nord") == 0) {
    *direction = 'N';
  } else if (strcasecmp(direction_str, "Sud") == 0) {
    *direction = 'S';
  } else {
    log_error("Direction invalide trouvée à la fin de la ligne: '%s'",
              direction_str);
    return false;
  }

  // --- 2. Tronquer la chaîne pour garder uniquement les coordonnées ---
  size_t len_coords = last_space - ligne;
  if (len_coords >= MAX_BUF_SIZE)
    len_coords = MAX_BUF_SIZE - 1;

  strncpy(coord_part, ligne, len_coords);
  coord_part[len_coords] = '\0';

  return true;
}

// ==========================================================================
// FONCTIONS PUBLIQUES
// ==========================================================================

/**
 * @brief Convertit une chaîne de coordonnées géographiques (ex: "47 deg 17 min
 * 48 sec Nord") en une valeur de latitude en double.
 *
 * NOTE: Cette fonction se concentre UNIQUEMENT sur le PARSING SYNTAXIQUE.
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
  char direction = ' ';
  char coord_part[MAX_BUF_SIZE];

  if (!ligne || !result) {
    log_error("Tentative de parsing avec ligne ou résultat NULL.");
    return ERR_CONVERSION;
  }

  // --- 1. Extraction (Responsabilité isolée) ---
  if (!extract_direction_and_coords(ligne, &direction, coord_part)) {
    return ERR_CONVERSION; // Échec de l'extraction
  }

  // --- 2. Parsing des composantes numériques ---
  // Format attendu : "XX deg YY min ZZ.ZZ sec" (secondes peuvent être décimales)
  int ret_sscanf =
      sscanf(coord_part, "%d deg %d min %lf sec", &degrees, &minutes, &seconds);

  if (ret_sscanf != 3) {
    log_debug("sscanf a échoué pour la partie '%s'. Retours: %d", coord_part,
              ret_sscanf);
    return ERR_CONVERSION; // Échec syntaxique du format numérique
  }

  // --- 3. Vérifications INTERNES des composantes (Logique de formatage) ---
  if (degrees < 0 || degrees > 90 || minutes < 0 || minutes >= 60 ||
      seconds < 0 || seconds >= 60) {
    log_error("Composantes de coordonnées hors plage de format (Deg: %d, Min: "
              "%d, Sec: %.2lf) trouvées dans '%s'.",
              degrees, minutes, seconds, coord_part);
    return ERR_CONVERSION; // Échec de validation interne du format
  }

  // --- 4. Conversion Finale ---
  double lat =
      (double)degrees + (double)minutes / 60.0 + (double)seconds / 3600.0;
  if (direction == 'S')
    lat = -lat;

  *result = lat;
  log_info("Latitude parsée avec succès: %f", lat);
  return ERR_OK; // Succès de la transformation syntaxique
}

/**
 * @brief Convertit une latitude en double en format Degrés° Minutes' Secondes".
 * @param lat La latitude en double.
 * @param buf Le buffer de sortie.
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
