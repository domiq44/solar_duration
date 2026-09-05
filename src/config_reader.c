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
    // Lookup table pour mappage paramètre → champ RawConfig
    typedef struct {
      const char *key;
      char **target_field;
    } ParamMapping;

    ParamMapping mappings[] = {
        {"annee", &raw->raw_annee_str},
        {"latitude", &raw->raw_latitude_str},
        {"jour_debut", &raw->raw_jour_debut_str},
        {"mois_debut", &raw->raw_mois_debut_str},
        {"jour_fin", &raw->raw_jour_fin_str},
        {"mois_fin", &raw->raw_mois_fin_str},
        {"mode_solaire", &raw->raw_mode_solaire_str},
        {NULL, NULL} // Sentinelle
    };

    // Parcourir la lookup table
    for (int i = 0; mappings[i].key != NULL; i++) {
      if (strcasecmp(param, mappings[i].key) == 0) {
        free(*mappings[i].target_field);
        *mappings[i].target_field = new_str;
        success_count++;
        break;
      }
    }
  }

  fclose(f);

  // Vérifier que tous les 7 paramètres obligatoires ont été trouvés
  if (success_count != 7) {
    log_error("Configuration incomplète: %d/7 paramètres trouvés. "
              "Paramètres requis: annee, latitude, jour_debut, mois_debut, "
              "jour_fin, mois_fin, mode_solaire",
              success_count);
    return READ_ERROR_DATA_MISSING;
  }
  return READ_SUCCESS;
}
