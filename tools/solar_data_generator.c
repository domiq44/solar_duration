// --- Fichier : tools/solar_data_generator.c ---

#include <stdio.h>

#include "config_reader.h"
#include "config_validator.h"
#include "date.h"
#include "logger.h"
#include "solar.h"

// --- Définition du fichier de sortie des données ---
#define OUTPUT_CSV "output/solar_model_comparison.csv"

typedef struct {
    double latitude;
    double altitude_threshold;
} SolarSettings;

static bool load_configured_solar_settings(SolarSettings *settings) {
    RawConfig *raw_config = create_raw_config();
    if (!raw_config) {
        return false;
    }

    FinalConfig config;
    bool success = read_raw_data("config/solar_duration.dat", raw_config) ==
                       READ_SUCCESS &&
                   parse_and_validate_config(raw_config, &config) ==
                       READ_SUCCESS &&
                   is_config_fully_valid(&config);
    if (success) {
        settings->latitude = config.latitude;
        settings->altitude_threshold = config.altitude_solaire_seuil_degres;
    }
    free_raw_config(raw_config);
    return success;
}

static double calculate_duration(double declination, double latitude,
                                 double altitude_threshold) {
    SolarObservation observation = {
        .declinaison_degres = declination,
        .latitude_degres = latitude,
        .altitude_solaire_seuil_degres = altitude_threshold,
    };
    return calculate_day_duration(observation);
}

// Écrit une ligne de données dans le CSV.
static bool write_model_data(int day, int month, int year, double delta_s,
                             double delta_sp, double delta_m, double latitude,
                             double altitude_threshold) {
    FILE *f = fopen(OUTPUT_CSV, "a");
    if (!f) {
        log_critical("Impossible d'ouvrir le fichier de sortie: %s", OUTPUT_CSV);
        return false;
    }

    int write_status = fprintf(
        f, "%02d/%02d/%d,%.4f,%.4f,%.4f,%.6f,%.8f,%.8f,%.8f,%.6f\n",
        day, month, year, delta_s, delta_sp, delta_m, latitude,
        calculate_duration(delta_s, latitude, altitude_threshold),
        calculate_duration(delta_sp, latitude, altitude_threshold),
        calculate_duration(delta_m, latitude, altitude_threshold),
        altitude_threshold);
    int close_status = fclose(f);
    if (write_status < 0 || close_status != 0) {
        log_critical("Impossible d'écrire dans le fichier de sortie: %s",
                     OUTPUT_CSV);
        return false;
    }
    return true;
}

// Génère les données des trois modèles pour une période fixe.
static bool generate_model_data(void) {
    SolarSettings settings;
    if (!load_configured_solar_settings(&settings)) {
        log_critical("Impossible de lire la configuration solaire.");
        return false;
    }

    log_info("========================================================");
    log_info("--- DÉBUT DE LA GÉNÉRATION DES DONNÉES SOLAIRES ---");
    log_info("========================================================");

    // Initialiser le fichier CSV.
    FILE *f = fopen(OUTPUT_CSV, "w");
    if (!f) {
        log_critical("Impossible de créer le fichier CSV: %s", OUTPUT_CSV);
        return false;
    }
    int header_status = fprintf(
        f, "Date,Delta_Sinusoidal,Delta_Spencer,Delta_Meeus,Latitude_Degrees,"
           "Duration_Sinusoidal_Hours,Duration_Spencer_Hours,"
           "Duration_Meeus_Hours,Solar_Altitude_Threshold_Degrees\n");
    int close_status = fclose(f);
    if (header_status < 0 || close_status != 0) {
        log_critical("Impossible d'écrire l'en-tête dans %s.", OUTPUT_CSV);
        return false;
    }

    // Générer les valeurs pour trois années complètes et reproductibles.
    const int start_year = 2024;
    const int end_year = 2026;
    CalendarDate current_date = {1, 1, start_year};

    while (current_date.annee <= end_year) {
        int ordinal_target = date_to_ordinal(current_date);

        if (ordinal_target != -1) {
            SolarDate solar_date = {
                .jour_ordinal = ordinal_target,
                .annee = current_date.annee,
            };
            double delta_s = calculate_sinusoidal_declination(solar_date);
            double delta_sp = calculate_spencer_declination(solar_date);
            double delta_m = calculate_meeus_declination(solar_date);
            if (!write_model_data(current_date.jour, current_date.mois,
                                  current_date.annee, delta_s, delta_sp,
                                  delta_m, settings.latitude,
                                  settings.altitude_threshold)) {
                return false;
            }
        }

        advance_day(&current_date);
    }

    log_info("Données enregistrées dans %s.", OUTPUT_CSV);
    return true;
}


int main(void) {
    if (!logger_init("config/solar_data_generator.cfg")) {
        fprintf(stderr, "ATTENTION: Logger non initialisé correctement.\n");
    }

    bool success = generate_model_data();

    log_info("========================================================");
    log_info("GÉNÉRATION DES DONNÉES SOLAIRES TERMINÉE.");
    log_info("========================================================");

    return success ? 0 : 1;
}
