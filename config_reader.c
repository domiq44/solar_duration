// --- File: config_reader.c ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_reader.h"
#include "logger.h"
#include "string_utils.h"

// ==========================================================================
// GESTION DE LA MÉMOIRE DES CONFIGURATIONS (RawConfig)
// ==========================================================================

RawConfig *create_raw_config() {
  RawConfig *raw = (RawConfig *)malloc(sizeof(RawConfig));
  if (!raw) {
    perror("Erreur d'allocation mémoire pour RawConfig");
    return NULL;
  }
  // Initialisation des pointeurs à NULL
  raw->raw_annee_str = NULL;
  raw->raw_latitude_str = NULL;
  raw->raw_jour_debut_str = NULL;
  raw->raw_mois_debut_str = NULL;
  raw->raw_jour_fin_str = NULL;
  raw->raw_mois_fin_str = NULL;
  raw->raw_mode_solaire_str = NULL;
  return raw;
}

/**
 * @brief Libère les chaînes allouées dans RawConfig puis libère la structure.
 */
void free_raw_config(RawConfig *raw) {
  if (!raw)
    return;

  // Libération des champs individuels
  free(raw->raw_annee_str);
  free(raw->raw_latitude_str);
  free(raw->raw_jour_debut_str);
  free(raw->raw_mois_debut_str);
  free(raw->raw_jour_fin_str);
  free(raw->raw_mois_fin_str);
  free(raw->raw_mode_solaire_str);

  // Libération de la structure elle-même
  free(raw);
}

// ==========================================================================
// LECTURE DES DONNÉES BRUTES (I/O)
// ==========================================================================

int read_raw_data(const char *filename, RawConfig *raw) {
  if (!raw) {
    log_error(
        "ERREUR: Le pointeur de configuration est NULL lors de la lecture.");
    return READ_ERROR_NULL_CONFIG;
  }

  FILE *f = fopen(filename, "r");
  if (!f) {
    log_error("ERREUR I/O: Impossible de trouver '%s'.", filename);
    return READ_ERROR_IO_FILE_NOT_FOUND;
  }

  char line[256];
  int success_count = 0;

  while (fgets(line, sizeof(line), f)) {
    // Gestion des commentaires (// ou #)
    char *comment_start = NULL;
    comment_start = strstr(line, "//");
    if (comment_start == NULL) {
      comment_start = strstr(line, "#");
    }
    if (comment_start) {
      *comment_start = '\0';
    }
    line[strcspn(line, "\n")] = '\0';

    char *eq = strchr(line, '=');
    if (!eq)
      continue;

    *eq = '\0';
    // Utilisation de strtrim depuis string_utils
    char *param = strtrim(line);
    const char *val = strtrim(eq + 1);

    // Allocation mémoire pour stocker la chaîne brute
    char *new_str = strdup(val);
    if (!new_str) {
      perror("Erreur d'allocation mémoire lors de la lecture");

      // NETTOYAGE EN CAS D'ERREUR
      // On utilise la fonction de nettoyage partielle pour ne pas détruire la
      // structure 'raw' si l'erreur survient avant que tous les champs n'aient
      // été remplis.
      free(raw->raw_annee_str);
      free(raw->raw_latitude_str);
      free(raw->raw_jour_debut_str);
      free(raw->raw_mois_debut_str);
      free(raw->raw_jour_fin_str);
      free(raw->raw_mois_fin_str);
      free(raw->raw_mode_solaire_str);

      fclose(f);
      return READ_ERROR_DATA_CONVERSION;
    }

    // Stockage basé sur le paramètre trouvé
    if (strcasecmp(param, "annee") == 0) {
      free(raw->raw_annee_str);
      raw->raw_annee_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "latitude") == 0) {
      free(raw->raw_latitude_str);
      raw->raw_latitude_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "jour_debut") == 0) {
      free(raw->raw_jour_debut_str);
      raw->raw_jour_debut_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "mois_debut") == 0) {
      free(raw->raw_mois_debut_str);
      raw->raw_mois_debut_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "jour_fin") == 0) {
      free(raw->raw_jour_fin_str);
      raw->raw_jour_fin_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "mois_fin") == 0) {
      free(raw->raw_mois_fin_str);
      raw->raw_mois_fin_str = new_str;
      success_count = 1;
    } else if (strcasecmp(param, "mode_solaire") == 0) {
      free(raw->raw_mode_solaire_str);
      raw->raw_mode_solaire_str = new_str;
      success_count = 1;
    }
  }

  fclose(f);

  if (success_count == 0) {
    return READ_ERROR_DATA_MISSING;
  }
  return READ_SUCCESS;
}
