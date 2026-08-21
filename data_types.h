// --- File: data_types.h ---

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include "solar.h"

// -------------------------------------------------------------------
// STRUCTURE POUR LES DONNÉES BRUTES (Lues du fichier)
// -------------------------------------------------------------------
typedef struct {
  char *raw_annee_str;
  char *raw_latitude_str;
  char *raw_jour_debut_str;
  char *raw_mois_debut_str;
  char *raw_jour_fin_str;
  char *raw_mois_fin_str;
  char *raw_mode_solaire_str;
} RawConfig;

// -------------------------------------------------------------------
// STRUCTURE POUR LES DONNÉES FINALES (Utilisables par la simulation)
// -------------------------------------------------------------------
typedef struct {
  int annee;
  double latitude;
  int jour_debut;
  int mois_debut;
  int jour_fin;
  int mois_fin;
  int mode_declinaison;
  DeclinationFunc declination_func;
} FinalConfig;

#endif // DATA_TYPES_H
