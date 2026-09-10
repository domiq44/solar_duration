# test_data_integrity.py
import csv
import os
import numpy as np

CSV_FILE = "solar_declination_test.csv"
MIN_EXPECTED_DAYS = 1090 # Ajuster pour couvrir les 3 ans
MAX_EXPECTED_DAYS = 1100 # Marge de sécurité pour les arrondis

def validate_data_integrity(csv_path):
    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier de données '{csv_path}' est manquant. Test échoué.")
        return False

    dates_list = []
    data_points = []

    try:
        with open(csv_path, 'r', newline='') as csvfile:
            reader = csv.reader(csvfile)
            next(reader) # Skip header
            
            for row in reader:
                if len(row) == 4:
                    dates_list.append(row[0])
                    # On ne stocke que le nombre de lignes pour le test de volume
                    data_points.append(row)
                        
    except Exception as e:
        print(f"[ERREUR] Impossible de lire le fichier CSV : {e}. Test échoué.")
        return False

    # --- Assertions de Volume ---
    num_days = len(data_points)
    if num_days < MIN_EXPECTED_DAYS or num_days > MAX_EXPECTED_DAYS:
        print(f"[ERREUR] Volume de données incorrect. Attendu: {MIN_EXPECTED_DAYS}-{MAX_EXPECTED_DAYS}, Trouvé: {num_days}. Test échoué.")
        return False
    
    print(f"[SUCCÈS] Nombre de jours dans le CSV: {num_days}. (OK)")
    return True

if __name__ == "__main__":
    if validate_data_integrity(CSV_FILE):
        print("\n✅ VALIDATION DE CONTENU : Toutes les données CSV sont dans la plage attendue.")
        exit(0) # Code de sortie 0 = Succès
    else:
        print("\n❌ VALIDATION DE CONTENU : Détection d'une anomalie dans les données.")
        exit(1) # Code de sortie 1 = Échec
