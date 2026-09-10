// --- tests/solar_test_runner.c ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "config_reader.h"
#include "config_validator.h"
#include "data_types.h"
#include "date.h"
#include "solar.h"
#include "logger.h"
#include "geo.h"

// --- Définition du fichier de sortie des données ---
#define TEST_OUTPUT_CSV "solar_declination_test.csv"

// Fonction pour écrire une ligne de données dans le CSV
void write_test_data(int day, int month, int year, 
                     double delta_s, double delta_sp, double delta_m) {
    FILE *f = fopen(TEST_OUTPUT_CSV, "a");
    if (f) {
        // Format: Date,Delta_Sinusoidal,Delta_Spencer,Delta_Meeus
        fprintf(f, "%02d/%02d/%d,%.4f,%.4f,%.4f\n", 
                day, month, year, delta_s, delta_sp, delta_m);
        fclose(f);
    } else {
        log_critical("Impossible d'ouvrir le fichier de sortie de test: %s", TEST_OUTPUT_CSV);
    }
}

// Fonction de simulation autonome (remplace la lecture de fichier)
void run_internal_solar_simulation(const FinalConfig *final) {
    log_info("========================================================");
    log_info("--- DÉBUT DE LA SIMULATION DE TEST SOLAIRE INTERNE ---");
    log_info("========================================================");

    // 1. Initialisation du fichier CSV
    FILE *f = fopen(TEST_OUTPUT_CSV, "w");
    if (f) {
        fprintf(f, "Date,Delta_Sinusoidal,Delta_Spencer,Delta_Meeus\n");
        fclose(f);
    } else {
        log_critical("ÉCHEC: Impossible de créer le fichier CSV de test: %s", TEST_OUTPUT_CSV);
        return;
    }

    // 2. Configuration de la boucle (Simulation sur 3 ans)
    // Nous allons simuler sur une plage fixe pour garantir la reproductibilité.
    int start_year = 2024; 
    int end_year = 2026; // 3 ans (2024, 2025, 2026)
    int start_day = 1;
    int start_month = 1;
    
    int current_day = start_day;
    int current_month = start_month;
    int current_year = start_year;
    
    // Boucle principale de simulation sur la plage définie
    while (current_year <= end_year) {
        
        // 1. Calcul de l'ordinal pour la date actuelle
        int ordinal_target = date_to_ordinal(current_day, current_month, current_year);
        
        if (ordinal_target != -1) {
            // 2. Calcul des trois déclinaisons
            double delta_s = calculate_sinusoidal_declination(ordinal_target, current_year);
            double delta_sp = calculate_spencer_declination(ordinal_target, current_year);
            double delta_m = calculate_meeus_declination(ordinal_target, current_year);

            // 3. Enregistrement des données
            write_test_data(current_day, current_month, current_year, 
                             delta_s, delta_sp, delta_m);
        }

        // 4. Avancer dans le temps
        advance_day(&current_day, &current_month, &current_year);
    }

    log_info("Simulation de test terminée. Données enregistrées dans %s.", TEST_OUTPUT_CSV);
}


int main(void) {
    // 1. Initialisation du Logger
    if (!logger_init("solar_test_runner.cfg")) {
        fprintf(stderr, "ATTENTION: Logger non initialisé correctement.\n");
    }

    // 2. Créer une configuration finale valide pour le test
    FinalConfig final;
    // NOTE: Nous utilisons une configuration générique pour que le test se lance
    // Nous devons simuler une configuration valide pour éviter les erreurs de validation.
    final.latitude = 45.0; // Latitude fixe pour le test
    final.mode_declinaison = MODE_SPENCER; // On force un mode pour la structure, mais on appelle les 3 fonctions
    // On initialise le pointeur pour éviter des crashs dans les fonctions de calcul
    final.declination_func = calculate_spencer_declination; 
    
    // 3. Lancement de la simulation interne
    run_internal_solar_simulation(&final);

    log_info("========================================================");
    log_info("TEST SOLAIRE TERMINÉ AVEC SUCCÈS.");
    log_info("========================================================");

    return 0;
}
