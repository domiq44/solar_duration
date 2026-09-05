// --- File: config_validator.h ---

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

/**
 * @brief Vérifie si la plage de dates est chronologiquement valide.
 * @param d_start Jour de début.
 * @param m_start Mois de début.
 * @param d_end Jour de fin.
 * @param m_end Mois de fin.
 * @param annee L'annee commune pour les deux bornes.
 * @return true si debut <= fin, false sinon.
 */
bool is_time_range_valid(int d_start, int m_start, int d_end, int m_end,
                         int annee);

/**
 * @brief Gère l'affichage d'un message d'erreur structuré en fonction du code.
 * @param read_status Le code d'erreur retourné par les fonctions de lecture.
 */
void handle_read_error(int read_status);

#endif // CONFIG_VALIDATOR_H
