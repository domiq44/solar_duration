// --- File: logger.h ---

#ifndef LOGGER_H
#define LOGGER_H

#include <stdbool.h>
#include <stdio.h>

// --- Définition des Niveaux de Log ---
// Plus la valeur est petite, plus le niveau est critique (et plus il est
// affiché par défaut)
typedef enum {
  LOG_DEBUG = 0, // Détails d'exécution (pour le débogage intense)
  LOG_INFO = 1,  // Opérations réussies (ex: "Simulation démarrée")
  LOG_WARN =
      2, // Potentiels problèmes ou déviations (ex: "Valeur hors plage, ignoré")
  LOG_ERROR = 3,   // Erreurs critiques nécessitant une intervention (ex: "Échec
                   // de lecture")
  LOG_CRITICAL = 4 // Erreurs fatales qui arrêtent le programme
} LogLevel;

// --- Configuration Globale du Log ---
// Le niveau minimum à afficher. Si le niveau d'un message est inférieur à ce
// seuil, il est ignoré.
extern LogLevel current_log_level;

// --- Fonctions d'API du Logger ---

/**
 * @brief Initialise le système de logging.
 * Si config_filename est NULL, utilise "logger.cfg" par défaut.
 * @param config_filename Le nom du fichier de configuration, ou NULL pour
 * utiliser le défaut.
 * @return bool true si l'initialisation a réussi.
 */
bool logger_init(const char *config_filename);

/**
 * @brief Enregistre un message de niveau DEBUG.
 */
void log_debug(const char *format, ...);

/**
 * @brief Enregistre un message d'information standard.
 */
void log_info(const char *format, ...);

/**
 * @brief Enregistre un avertissement (potentiel problème).
 */
void log_warn(const char *format, ...);

/**
 * @brief Enregistre une erreur non fatale (le processus peut continuer).
 */
void log_error(const char *format, ...);

/**
 * @brief Enregistre une erreur critique (le processus doit s'arrêter).
 * @param format Le message d'erreur.
 * @param ... Arguments formatés.
 * @return 1 si l'arrêt est nécessaire, 0 sinon.
 */
int log_critical(const char *format, ...);

#endif // LOGGER_H
