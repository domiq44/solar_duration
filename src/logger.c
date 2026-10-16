// --- Fichier : logger.c ---

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "logger.h"
#include "string_utils.h"

// --- Configuration interne ---
LogLevel current_log_level = LOG_INFO; // Valeur par défaut
#define DEFAULT_LOG_CONFIG_FILE "config/solar_duration.cfg"

// --- Fonction auxiliaire pour obtenir l'heure actuelle sous forme de chaîne ---

/**
 * @brief Récupère l'heure système actuelle et la formate dans un buffer
 * spécifié.
 *
 * Cette fonction utilise les fonctions temporelles de la bibliothèque C
 * standard pour obtenir le moment actuel, le convertir en structure locale,
 * puis le formater en une chaîne de caractères lisible.
 *
 * @param buffer Le pointeur vers le buffer de sortie où le timestamp sera
 * stocké. Ce buffer doit avoir une taille suffisante pour contenir le format
 * cible (ex: 32 caractères pour "%Y-%m-%d %H:%M:%S").
 * @param size La taille maximale du buffer fourni. Assure que strftime ne
 * déborde pas.
 *
 * @pre Le buffer doit être initialisé et avoir une taille >= 32.
 * @post Le buffer contiendra une chaîne formatée de type "AAAA-MM-JJ HH:MM:SS".
 * @note Si l'opération échoue (par exemple, si le buffer est trop petit),
 *       le comportement est imprévisible, mais la fonction ne gère pas l'erreur
 *       dans ce cas (elle dépend de la vérification de la taille par
 * l'appelant).
 */
static void get_timestamp(char *buffer, size_t size) {
  time_t timer;
  const struct tm *tm_info;
  time(&timer);
  tm_info = localtime(&timer);
  strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
}

/**
 * @brief Convertit un LogLevel en sa représentation textuelle.
 * @param level Le niveau à convertir.
 * @return Une chaîne de caractères représentant le niveau (ex: "INFO").
 */
static const char *level_to_string(LogLevel level) {
  switch (level) {
  case LOG_DEBUG:
    return "DEBUG";
  case LOG_INFO:
    return "INFO";
  case LOG_WARN:
    return "WARN";
  case LOG_ERROR:
    return "ERROR";
  case LOG_CRITICAL:
    return "CRITICAL";
  default:
    return "UNKNOWN";
  }
}

// --- Logique de Lecture du Fichier de Configuration ---

/**
 * @brief Tente de lire le niveau de log depuis un fichier spécifié.
 * @param filename Le nom du fichier à lire.
 * @return true si le niveau a été lu avec succès, false sinon.
 */
static bool load_log_level_from_config(const char *filename) {
  current_log_level = LOG_INFO;

  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr,
            "[SYSTEM] Fichier de configuration de log ('%s') non trouvé. "
            "Utilisation du niveau par défaut (INFO).\n",
            filename);
    return false;
  }

  char line[128];
  LogLevel loaded_level = LOG_INFO;
  int found = 0;

  while (fgets(line, sizeof(line), f)) {
    // Nettoyage de la ligne
    char *trimmed_line = line;
    while (*trimmed_line == ' ' || *trimmed_line == '\t')
      trimmed_line++;
    if (*trimmed_line == '\0' || *trimmed_line == '#')
      continue; // Ignorer lignes vides ou commentaires

    // Recherche du format "level = X"
    char *eq = strchr(trimmed_line, '=');
    if (eq) {
      *eq = '\0';
      const char *key = strtrim(trimmed_line);
      const char *val_str = strtrim(eq + 1);

      if (strcmp(key, "level") != 0) {
        continue;
      }

      // Table de correspondance entre les chaînes et les niveaux de journalisation
      typedef struct {
        const char *name;
        LogLevel level;
      } LevelMapping;

      LevelMapping levels[] = {
          {"DEBUG", LOG_DEBUG},
          {"INFO", LOG_INFO},
          {"WARN", LOG_WARN},
          {"ERROR", LOG_ERROR},
          {"CRITICAL", LOG_CRITICAL},
          {NULL, LOG_INFO} // Sentinelle avec valeur par défaut
      };

      // Parcourir la table de correspondance
      for (int i = 0; levels[i].name != NULL; i++) {
        if (strcmp(val_str, levels[i].name) == 0) {
          loaded_level = levels[i].level;
          found = 1;
          break;
        }
      }

      if (found)
        break; // Sortir de la boucle fgets
    }
  }

  fclose(f);

  if (found) {
    current_log_level = loaded_level;

    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    fprintf(stdout,
            "[%s] [INFO] Niveau de log chargé avec succès depuis '%s' : %s\n",
            timestamp, filename, level_to_string(current_log_level));
    return true;
  } else {
    fprintf(stderr,
            "[SYSTEM] Aucun niveau de log valide trouvé dans '%s'. Utilisation "
            "du niveau par défaut (INFO).\n",
            filename);
    return false;
  }
}

// --- Implémentations des fonctions publiques ---

/**
 * @brief Initialise le système de logging.
 * Si config_filename est NULL, utilise "logger.cfg" par défaut.
 * @param config_filename Le nom du fichier de configuration, ou NULL pour
 * utiliser le défaut.
 * @return bool true si l'initialisation a réussi.
 */
bool logger_init(const char *config_filename) {
  // 1. Détermination du nom de fichier à utiliser
  const char *filename_to_use;
  if (config_filename == NULL) {
    filename_to_use = DEFAULT_LOG_CONFIG_FILE;
  } else {
    filename_to_use = config_filename;
  }

  // 2. Appel de la fonction de chargement avec le nom déterminé
  return load_log_level_from_config(filename_to_use);
}

/**
 * @brief Fonction interne qui gère l'affichage du message en fonction du
 * niveau.
 * @param level Le niveau du message reçu.
 * @param format La chaîne de format.
 * @param args Les arguments formatés.
 * @param output_stream Le flux où écrire (stdout ou stderr).
 */
static void log_internal(LogLevel level, const char *format, va_list args,
                         FILE *output_stream) {
  if (level < current_log_level) {
    return;
  }

  char timestamp[32];
  get_timestamp(timestamp, sizeof(timestamp));

  fprintf(output_stream, "[%s] [%s]: ", timestamp, level_to_string(level));

  va_list args_copy;
  va_copy(args_copy, args);
  // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
  vfprintf(output_stream, format, args_copy);
  va_end(args_copy);
  fprintf(output_stream, "\n");
  fflush(output_stream);
}

static void log_result_internal(const char *format, va_list args) {
  char timestamp[32];
  get_timestamp(timestamp, sizeof(timestamp));

  fprintf(stdout, "[%s] [RESULT]: ", timestamp);

  va_list args_copy;
  va_copy(args_copy, args);
  // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
  vfprintf(stdout, format, args_copy);
  va_end(args_copy);
  fprintf(stdout, "\n");
  fflush(stdout);
}

// --- Implémentations des fonctions publiques ---

void log_debug(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_internal(LOG_DEBUG, format, args, stderr);
  va_end(args);
}

void log_info(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_internal(LOG_INFO, format, args, stdout);
  va_end(args);
}

void log_result(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_result_internal(format, args);
  va_end(args);
}

void log_warn(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_internal(LOG_WARN, format, args, stderr);
  va_end(args);
}

void log_error(const char *format, ...) {
  va_list args;
  va_start(args, format);
  log_internal(LOG_ERROR, format, args, stderr);
  va_end(args);
}

int log_critical(const char *format, ...) {
  va_list args;
  va_start(args, format);
  // Affichage critique toujours sur stderr
  fprintf(stderr,
          "\n========================================================\n");
  fprintf(stderr, "[CRITICAL] ");
  va_list args_copy;
  va_copy(args_copy, args);
  // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
  vfprintf(stderr, format, args_copy);
  va_end(args_copy);
  fprintf(stderr,
          "\n========================================================\n");
  va_end(args);
  return 1; // Indique l'arrêt
}
