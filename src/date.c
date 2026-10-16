// --- Fichier : date.c ---

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "date.h"

// ==========================================================================
// FONCTION UTILITAIRE : Détermination du nombre de jours dans un mois
// ==========================================================================

/**
 * @brief Retourne le nombre de jours dans un mois donné pour une année
 * spécifique.
 *
 * Cette fonction encapsule la logique de base des jours dans les mois, y
 * compris la gestion des années bissextiles pour février.
 *
 * @param month Le mois (valeur de 1 à 12).
 * @param year L'année concernée.
 * @return int Le nombre de jours dans le mois. Retourne 0 si le mois est
 * invalide.
 */
static int get_days_in_month(int month, int year) {
  // Tableau de base (jours par mois, non bissextile)
  // Index 0 est ignoré, les indices 1 à 12 correspondent aux mois.
  const int days_in_month[] = {0,  31, 28, 31, 30, 31, 30,
                               31, 31, 30, 31, 30, 31};

  if (month < 1 || month > 12) {
    return 0; // Mois invalide
  }

  // Correction pour février dans les années bissextiles
  if (month == 2 && is_leap_year(year)) {
    return 29;
  }

  return days_in_month[month];
}

// ==========================================================================
// FONCTION UTILITAIRE : Bissextile
// ==========================================================================

/**
 * @brief Vérifie si une année donnée est bissextile.
 * @param annee L'année à tester.
 * @return true si l'année est bissextile, false sinon.
 */
int is_leap_year(int annee) {
  return (annee % 4 == 0 && (annee % 100 != 0 || annee % 400 == 0));
}

// ==========================================================================
// FONCTION DE CONVERSION : Date -> Jour Ordinal
// ==========================================================================

/**
 * @brief Convertit une combinaison Jour/Mois/Année en son Jour Ordinal de
 * l'année (1 à 366).
 *
 * @param day Jour du mois.
 * @param month Mois de l'année.
 * @param year Année.
 * @return int Le jour ordinal si la date est valide, -1 sinon.
 */
int date_to_ordinal(CalendarDate date) {
  // Validation initiale des plages de base
  if (date.mois < 1 || date.mois > 12 || date.jour < 1) {
    return -1; // Échec de validation de base (Mois ou Jour trop petit)
  }

  // Validation stricte : Vérifie si le jour existe dans le mois donné
  int max_days = get_days_in_month(date.mois, date.annee);
  if (date.jour > max_days) {
    return -1; // Jour invalide pour ce mois/année (ex: 31 février)
  }

  int ordinal = 0;
  // Somme des jours des mois précédents
  for (int month = 1; month < date.mois; month++) {
    ordinal += get_days_in_month(month, date.annee);
  }

  ordinal += date.jour;
  return ordinal;
}

/**
 * @brief Vérifie si une combinaison Jour/Mois/Année est valide.
 * @param day Jour.
 * @param month Mois.
 * @param year Année.
 * @return true si la date est valide (y compris la vérification du jour dans le
 * mois), false sinon.
 */
bool is_date_valid(CalendarDate date) {
  // date_to_ordinal retourne -1 si la validation de base échoue.
  return date_to_ordinal(date) != -1;
}

bool is_date_range_ordered(DateRange range) {
  int start_ordinal = date_to_ordinal(range.debut);
  int end_ordinal = date_to_ordinal(range.fin);

  if (start_ordinal == -1 || end_ordinal == -1) {
    return false;
  }
  if (range.debut.annee != range.fin.annee) {
    return range.debut.annee < range.fin.annee;
  }
  return start_ordinal <= end_ordinal;
}

// ==========================================================================
// FONCTION UTILITAIRE : Avancer le jour
// ==========================================================================

/**
 * @brief Avance la date de un jour.
 *
 * Met à jour les pointeurs jour, mois et année pour représenter le jour
 * suivant.
 *
 * @param jour Pointeur vers le jour actuel (sera incrémenté).
 * @param mois Pointeur vers le mois actuel (sera incrémenté si débordement de
 * jour).
 * @param annee Pointeur vers l'année actuelle (sera incrémenté si débordement
 * de mois).
 */
void advance_day(CalendarDate *date) {
  // 1. Calculer le nombre de jours dans le mois actuel en utilisant
  // l'utilitaire
  int days_in_month = get_days_in_month(date->mois, date->annee);

  // 2. Avancer le jour
  date->jour++;

  // 3. Gestion du débordement de mois
  if (date->jour > days_in_month) {
    date->jour = 1;
    date->mois++;

    // 4. Gestion du débordement d'année
    if (date->mois > 12) {
      date->mois = 1;
      date->annee++;
    }
  }
}
