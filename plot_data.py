import matplotlib.pyplot as plt
import csv 
import os
import numpy as np # <--- C'EST LA CORRECTION CRUCIALE
from datetime import datetime

CSV_FILE = "simulation_data.csv"

def plot_duration_vs_date(csv_path):
    """
    Charge les données du CSV en utilisant le module CSV standard de Python.
    """
    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier de données '{csv_path}' est manquant. Veuillez exécuter 'make run_log' d'abord.")
        return

    dates_list = []
    durations_list = []

    try:
        with open(csv_path, 'r', newline='') as csvfile:
            reader = csv.reader(csvfile)
            
            # Sauter l'en-tête
            next(reader) 
            
            for row in reader:
                if len(row) == 2:
                    date_str = row[0]
                    try:
                        duration = float(row[1])
                        dates_list.append(date_str)
                        durations_list.append(duration)
                    except ValueError:
                        print(f"[AVERTISSEMENT] Impossible de convertir la durée pour la date {date_str}. Ignorée.")
                        continue
                        
    except Exception as e:
        print(f"[ERREUR] Impossible de lire le fichier CSV : {e}")
        return

    # --- Configuration du Graphique ---
    plt.figure(figsize=(14, 7))
    
    # Tracer la ligne de tendance
    plt.plot(dates_list, durations_list, marker='o', linestyle='-', color='skyblue', label='Durée du Jour (Heures)')
    
    # Ajouter une ligne horizontale pour la moyenne (Maintenant fonctionnelle grâce à np)
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
    output_filename = "daily_duration_plot.png"
    plt.savefig(output_filename)
    print(f"\n✅ SUCCÈS : Le graphique a été généré et sauvegardé sous '{output_filename}'")

if __name__ == "__main__":
    plot_duration_vs_date(CSV_FILE)
