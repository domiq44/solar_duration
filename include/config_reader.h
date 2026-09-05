// --- File: config_reader.h ---

#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stdbool.h>

#include "data_types.h"

// Définitions des codes d'erreur
#define READ_SUCCESS (0)
#define READ_ERROR_NULL_CONFIG (-1)
#define READ_ERROR_IO_FILE_NOT_FOUND (-2)
#define READ_ERROR_DATA_CONVERSION (-3)
#define READ_ERROR_DATA_MISSING (-4)

/**
 * @brief Alloue et initialise une structure RawConfig vide.
 * @return RawConfig* Le pointeur vers la structure allouée, ou NULL en cas
 * d'échec.
 */
RawConfig *create_raw_config();

/**
 * @brief Libère toutes les chaînes allouées dans RawConfig et la structure
 * elle-même.
 * @param raw Le pointeur vers la structure à nettoyer.
 */
void free_raw_config(RawConfig *raw);

/**
 * @brief Lit le fichier de configuration et peuple la structure RawConfig.
 * @param filename Le nom du fichier à lire.
 * @param raw Le pointeur où stocker les données brutes.
 * @return int Le code de succès/erreur.
 */
int read_raw_data(const char *filename, RawConfig *raw);

#endif // CONFIG_READER_H
