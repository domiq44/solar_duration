// --- Fichier : config_reader.c ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "config_reader.h"
#include "logger.h"
#include "string_utils.h"

// ==========================================================================
// GESTION DE LA MÉMOIRE DES CONFIGURATIONS (RawConfig)
// ==========================================================================

static void clear_raw_config_fields(RawConfig *raw) {
  free(raw->raw_annee_debut_str);
  free(raw->raw_annee_fin_str);
  free(raw->raw_latitude_str);
  free(raw->raw_jour_debut_str);
  free(raw->raw_mois_debut_str);
  free(raw->raw_jour_fin_str);
  free(raw->raw_mois_fin_str);
  free(raw->raw_mode_solaire_str);
  free(raw->raw_altitude_solaire_seuil_str);

  raw->raw_annee_debut_str = NULL;
  raw->raw_annee_fin_str = NULL;
  raw->raw_latitude_str = NULL;
  raw->raw_jour_debut_str = NULL;
  raw->raw_mois_debut_str = NULL;
  raw->raw_jour_fin_str = NULL;
  raw->raw_mois_fin_str = NULL;
  raw->raw_mode_solaire_str = NULL;
  raw->raw_altitude_solaire_seuil_str = NULL;
}

RawConfig *create_raw_config(void) {
  RawConfig *raw = (RawConfig *)malloc(sizeof(RawConfig));
  if (!raw) {
    perror("Erreur d'allocation mémoire pour RawConfig");
    return NULL;
  }
  // Initialisation des pointeurs à NULL
  raw->raw_annee_debut_str = NULL;
  raw->raw_annee_fin_str = NULL;
  raw->raw_latitude_str = NULL;
  raw->raw_jour_debut_str = NULL;
  raw->raw_mois_debut_str = NULL;
  raw->raw_jour_fin_str = NULL;
  raw->raw_mois_fin_str = NULL;
  raw->raw_mode_solaire_str = NULL;
  raw->raw_altitude_solaire_seuil_str = NULL;
  return raw;
}

/**
 * @brief Libère les chaînes allouées dans RawConfig puis libère la structure.
 */
void free_raw_config(RawConfig *raw) {
  if (!raw)
    return;

  clear_raw_config_fields(raw);

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

  clear_raw_config_fields(raw);

  FILE *f = fopen(filename, "r");
  if (!f) {
    log_error("ERREUR I/O: Impossible de trouver '%s'.", filename);
    return READ_ERROR_IO_FILE_NOT_FOUND;
  }

  char line[256];
  int reached_eof = 0;
  while (!reached_eof && fgets(line, sizeof(line), f)) {
    if (strchr(line, '\n') == NULL) {
      int next_character = fgetc(f);
      if (next_character == EOF && ferror(f)) {
        log_error("Erreur de lecture dans '%s'.", filename);
        clear_raw_config_fields(raw);
        fclose(f);
        return READ_ERROR_IO_READ;
      }
      if (next_character == EOF) {
        reached_eof = 1;
      } else if (next_character != '\n') {
        while (next_character != '\n' && next_character != EOF) {
          next_character = fgetc(f);
        }
        log_error("Ligne de configuration trop longue dans '%s'.", filename);
        clear_raw_config_fields(raw);
        fclose(f);
        return READ_ERROR_DATA_CONVERSION;
      }
    }

    // Gestion des commentaires (// ou #)
    char *comment_start = NULL;
    comment_start = strstr(line, "//");
    if (comment_start == NULL) {
      comment_start = strstr(line, "#");
    }
    if (comment_start) {
      *comment_start = '\0';
    }
    char *newline = strchr(line, '\n');
    if (newline) {
      *newline = '\0';
    }

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

      clear_raw_config_fields(raw);

      fclose(f);
      return READ_ERROR_DATA_CONVERSION;
    }

    // Stockage basé sur le paramètre trouvé
    // Table de correspondance entre les paramètres et les champs de RawConfig
    typedef struct {
      const char *key;
      char **target_field;
    } ParamMapping;

    ParamMapping mappings[] = {
        {"annee_debut", &raw->raw_annee_debut_str},
        {"annee_fin", &raw->raw_annee_fin_str},
        {"latitude", &raw->raw_latitude_str},
        {"jour_debut", &raw->raw_jour_debut_str},
        {"mois_debut", &raw->raw_mois_debut_str},
        {"jour_fin", &raw->raw_jour_fin_str},
        {"mois_fin", &raw->raw_mois_fin_str},
        {"mode_solaire", &raw->raw_mode_solaire_str},
        {"altitude_solaire_seuil", &raw->raw_altitude_solaire_seuil_str},
        {NULL, NULL} // Sentinelle
    };

    int matched = 0;
    for (int i = 0; mappings[i].key != NULL; i++) {
      if (strcasecmp(param, mappings[i].key) == 0) {
        free(*mappings[i].target_field);
        *mappings[i].target_field = new_str;
        matched = 1;
        break;
      }
    }
    if (!matched) {
      free(new_str);
    }
  }

  if (ferror(f)) {
    log_error("Erreur de lecture dans '%s'.", filename);
    clear_raw_config_fields(raw);
    fclose(f);
    return READ_ERROR_IO_READ;
  }

  fclose(f);

  // Vérifier que tous les 8 paramètres obligatoires ont été trouvés
  int populated_count = (raw->raw_annee_debut_str != NULL) +
                        (raw->raw_annee_fin_str != NULL) +
                        (raw->raw_latitude_str != NULL) +
                        (raw->raw_jour_debut_str != NULL) +
                        (raw->raw_mois_debut_str != NULL) +
                        (raw->raw_jour_fin_str != NULL) +
                        (raw->raw_mois_fin_str != NULL) +
                        (raw->raw_mode_solaire_str != NULL);
  if (populated_count != 8) {
    log_error("Configuration incomplète: %d/8 paramètres trouvés. "
              "Paramètres requis: annee_debut, mois_debut, jour_debut, "
              "annee_fin, mois_fin, jour_fin, latitude, mode_solaire",
              populated_count);
    return READ_ERROR_DATA_MISSING;
  }
  return READ_SUCCESS;
}
