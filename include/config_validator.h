// --- Fichier : config_validator.h ---

#ifndef CONFIG_VALIDATOR_H
#define CONFIG_VALIDATOR_H

#include <stdbool.h>

#include "data_types.h"

// Codes d'erreur (réutilisés pour la clarté)
// #define READ_SUCCESS (0) ...

/**
 * @brief Convertit les données brutes en types finaux et effectue la validation
 * de format.
 * @param raw La configuration brute.
 * @param final Le pointeur où stocker les données finales.
 * @return int Le code de succès/erreur.
 */
int parse_and_validate_config(const RawConfig *raw, FinalConfig *final);

/**
 * @brief Vérifie si la configuration finale est structurellement et logiquement
 * valide.
 * @param final La configuration finale.
 * @return true si tout est valide, false sinon.
 */
bool is_config_fully_valid(const FinalConfig *final);

#endif // CONFIG_VALIDATOR_H
