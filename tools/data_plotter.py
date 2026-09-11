# --- File: plot_data.py ---
import matplotlib.pyplot as plt
import csv 
import os
import numpy as np
from datetime import datetime

# --- Paramètre de contrôle de la densité des points sur l'axe X ---
# Un intervalle de 21 signifie qu'on ne garde qu'un point tous les 21 jours.
DATA_SAMPLING_INTERVAL = 21
# ------------------------------------------------------------------

CSV_FILE = "solar_duration.csv"

def plot_duration_vs_date(csv_path):
    """
    Charge les données du CSV, applique l'échantillonnage, et génère un graphique.
    """
    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier de données '{csv_path}' est manquant. Veuillez exécuter 'make run_log' d'abord.")
        return

    # Stockage des données brutes (avant échantillonnage)
    raw_dates_list = []
    raw_durations_list = []

    print(f"[*] Lecture des données brutes depuis '{csv_path}'...")
    
    # --- PHASE 1 : Lecture brute du fichier CSV ---
    try:
        with open(csv_path, 'r', newline='') as csvfile:
            reader = csv.reader(csvfile)
            
            # Sauter l'en-tête
            try:
                next(reader) 
            except StopIteration:
                print("[AVERTISSEMENT] Le fichier CSV est vide.")
                return

            for row in reader:
                if len(row) == 2:
                    date_str = row[0]
                    try:
                        duration = float(row[1])
                        # Stockage des données brutes
                        raw_dates_list.append(date_str)
                        raw_durations_list.append(duration)
                    except ValueError:
                        print(f"[AVERTISSEMENT] Impossible de convertir la durée pour la date {date_str}. Ignorée.")
                        continue
                        
    except Exception as e:
        print(f"[ERREUR] Impossible de lire le fichier CSV : {e}")
        return
    
    print(f"[*] Données brutes chargées : {len(raw_dates_list)} points.")


    # --- PHASE 2 : ÉCHANTILLONNAGE DES DONNÉES (Implémentation demandée) ---
    if len(raw_dates_list) > DATA_SAMPLING_INTERVAL:
        step = DATA_SAMPLING_INTERVAL 
        
        print(f"\n[*] ÉCHANTILLONNAGE ACTIF : Réduction des points de {len(raw_dates_list)} à environ {len(raw_dates_list) // step} points.")
        
        # Application de l'échantillonnage sur les listes
        dates_list = raw_dates_list[::step]
        durations_list = raw_durations_list[::step]
    else:
        # Si les données sont déjà peu nombreuses, on utilise toutes les données brutes
        print("\n[*] ÉCHANTILLONNAGE INACTIF : Le nombre de points est inférieur ou égal à l'intervalle. Toutes les données sont utilisées.")
        dates_list = raw_dates_list
        durations_list = raw_durations_list
        
    
    # --- PHASE 3 : Configuration et Sauvegarde du Graphique ---
    if not dates_list:
        print("[ERREUR] Aucune donnée valide à tracer après l'échantillonnage.")
        return

    plt.figure(figsize=(14, 7))
    
    # Tracer la ligne de tendance
    plt.plot(dates_list, durations_list, marker='o', linestyle='-', color='skyblue', label='Durée du Jour (Heures)')
    
    # Ajouter une ligne horizontale pour la moyenne
    mean_duration = np.mean(durations_list) if durations_list else 0
    plt.axhline(y=mean_duration, color='r', linestyle='--', label=f'Moyenne ({mean_duration:.2f}h)')

    # Configuration des axes et titres
    plt.title("Durée du Jour en Fonction de la Date (Simulation Solaire)", fontsize=16)
    plt.xlabel("Date (JJ/MM/AAAA)", fontsize=12)
    plt.ylabel("Durée du Jour (Heures)", fontsize=12)
    
    plt.xticks(rotation=45, ha='right')
    
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend()
    plt.tight_layout()
    
    # Sauvegarder le graphique
    output_filename = "solar_duration.png"
    plt.savefig(output_filename)
    print(f"\n✅ SUCCÈS : Le graphique a été généré et sauvegardé sous '{output_filename}'")

if __name__ == "__main__":
    plot_duration_vs_date(CSV_FILE)
