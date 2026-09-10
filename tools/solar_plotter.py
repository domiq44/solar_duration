import matplotlib.pyplot as plt
import os

CSV_FILE = "solar_declination_test.csv"

# --- Paramètre de contrôle de la densité des points sur l'axe X ---
# Un intervalle de 7 signifie qu'on ne garde qu'un point tous les 7 jours.
# Si vous voulez moins de points, augmentez ce nombre (ex: 14, 21).
DATA_SAMPLING_INTERVAL = 21
# ------------------------------------------------------------------

# Contrôle de la verbosité
VERBOSE_MODE = os.environ.get("SOLAR_DEBUG", "False").lower() == "true"

def plot_declination_curves(csv_path):
    """
    Charge les trois courbes de déclinaison, échantillonne les données, et génère un graphique.
    """
    if not os.path.exists(csv_path):
        print(f"[ERREUR] Le fichier de données '{csv_path}' est manquant. Veuillez exécuter 'make run_solar_test' d'abord.")
        return

    # Stockage des données brutes (avant échantillonnage)
    raw_data = {
        'date_sin': [], 'date_sp': [], 'date_m': [],
        'delta_sin': [], 'delta_sp_list': [], 'delta_m': []
    }
    
    if VERBOSE_MODE:
        print(f"[*] DÉBUT DU DÉBOGAGE : Lecture brute du fichier de test : {csv_path}...")

    try:
        with open(csv_path, 'r') as f:
            lines = f.readlines()
            
            if not lines:
                print("[ERREUR] Le fichier CSV est vide.")
                return
            
            data_lines = lines[1:]
            
            for i, line in enumerate(data_lines):
                line = line.strip()
                
                if not line:
                    continue

                if VERBOSE_MODE:
                    print(f"\n--- LIGNE {i + 2} ---")
                    print(f"Contenu brut: '{line}'")

                parts = line.split(',')
                
                if len(parts) != 4:
                    if VERBOSE_MODE:
                        print(f"[AVERTISSEMENT] Ligne ignorée car elle ne contient pas 4 champs: {line}")
                    continue
                
                try:
                    date_str = parts[0].strip()
                    delta_s = float(parts[1].strip())
                    delta_sp_val = float(parts[2].strip())
                    delta_m_val = float(parts[3].strip())
                    
                    # Stockage des données brutes
                    raw_data['date_sin'].append(date_str)
                    raw_data['date_sp'].append(date_str)
                    raw_data['date_m'].append(date_str)
                    raw_data['delta_sin'].append(delta_s)
                    raw_data['delta_sp_list'].append(delta_sp_val)
                    raw_data['delta_m'].append(delta_m_val)
                        
                except ValueError:
                    if VERBOSE_MODE:
                        print(f"[AVERTISSEMENT] ERREUR DE CONVERSION FLOAT sur la ligne {i + 2}: {line}. Ignorée.")
                    continue
                        
    except Exception as e:
        print(f"[ERREUR FATALE] Une erreur générale est survenue lors de la lecture du fichier CSV : {e}")
        return

    # ====================================================================
    # *** NOUVELLE ÉTAPE : ÉCHANTILLONNAGE DES DONNÉES ***
    # Nous ne conservons que les données à l'intervalle défini (ex: tous les 7 jours)
    
    if len(raw_data['date_sin']) > DATA_SAMPLING_INTERVAL:
        # Utilise slicing pour garder un échantillon régulier
        step = DATA_SAMPLING_INTERVAL 
        
        print(f"\n[*] ÉCHANTILLONNAGE ACTIF : Réduction des points de {len(raw_data['date_sin'])} à environ {len(raw_data['date_sin']) // step} points.")
        
        date_sin = raw_data['date_sin'][::step]
        date_sp = raw_data['date_sp'][::step]
        date_m = raw_data['date_m'][::step]
        
        delta_sin = raw_data['delta_sin'][::step]
        delta_sp_list = raw_data['delta_sp_list'][::step]
        delta_m = raw_data['delta_m'][::step]
    else:
        # Si les données sont déjà peu nombreuses, on utilise toutes les données brutes
        date_sin = raw_data['date_sin']
        date_sp = raw_data['date_sp']
        date_m = raw_data['date_m']
        delta_sin = raw_data['delta_sin']
        delta_sp_list = raw_data['delta_sp_list']
        delta_m = raw_data['delta_m']


    # --- Configuration du Graphique ---
    plt.figure(figsize=(16, 8))
    
    # Tracé avec les données échantillonnées
    plt.plot(date_sin, delta_sin, marker='.', linestyle='-', label='Sinusoïdal (Mode 1)', alpha=0.7)
    plt.plot(date_sp, delta_sp_list, marker='.', linestyle='-', label='Spencer (Mode 2)', alpha=0.7)
    plt.plot(date_m, delta_m, marker='.', linestyle='-', label='Meeus (Mode 3)', alpha=0.7)

    plt.title("Comparaison des Modèles de Déclinaison Solaire", fontsize=18)
    plt.xlabel("Date (JJ/MM/AAAA)", fontsize=14)
    plt.ylabel("Déclinaison Solaire (Degrés)", fontsize=14)
    
    # Utilisation de la rotation pour gérer les chaînes de caractères
    plt.xticks(rotation=45, ha='right')
    
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.legend()
    plt.tight_layout()
    
    output_filename = "solar_declination_comparison_plot.png"
    plt.savefig(output_filename)
    print(f"\n✅ SUCCÈS : Le graphique de comparaison a été généré et sauvegardé sous '{output_filename}'")

if __name__ == "__main__":
    plot_declination_curves(CSV_FILE)
