import re
import sys
import os

LOG_FILE = "solar_duration.log"

def parse_log_data(log_path):
    """
    Lit le fichier de log et extrait les tuples (date, duree_en_heures) en utilisant
    une recherche globale (findall) pour une efficacité maximale.
    """
    data = []
    
    # --- REGEX MIS À JOUR POUR ÊTRE PLUS ROBUSTE ET CAPTURER TOUT ---
    # Capture (Date) ET (Heure) à partir de la ligne de simulation.
    simulation_pattern = re.compile(
        r"(\d{2}/\d{2}/\d{4}).*?->\s*(\d{2}:\d{2}:\d{2})"
    )
    
    try:
        with open(log_path, 'r') as f:
            print(f"[*] Lecture du fichier de log : {log_path}...")
            
            # Lire tout le contenu en une seule chaîne
            content = f.read()
            
            # Utiliser findall pour obtenir TOUTES les correspondances
            matches = simulation_pattern.findall(content)
            
            if not matches:
                print("[ERREUR] Aucune ligne de simulation trouvée dans le log.")
                return []

            for date_str, time_str in matches:
                # Conversion du temps HH:MM:SS en heures décimales
                try:
                    hours, minutes, seconds = map(int, time_str.split(':'))
                    total_hours = hours + (minutes / 60.0) + (seconds / 3600.0)
                    data.append((date_str, total_hours))
                except ValueError:
                    # Si la conversion échoue (devrait pas arriver ici), on passe
                    continue
                        
        print(f"[+] Succès : {len(data)} jours de données extraits.")
        return data
        
    except FileNotFoundError:
        print(f"[ERREUR] Le fichier de log '{log_path}' est introuvable. Avez-vous exécuté 'make run_log' ?")
        sys.exit(1)
    except Exception as e:
        print(f"[ERREUR] Une erreur inattendue est survenue lors de la lecture : {e}")
        sys.exit(1)

if __name__ == "__main__":
    parsed_data = parse_log_data(LOG_FILE)
    
    if not parsed_data:
        print("!!! ATTENTION : Aucune donnée n'a été parsée. Vérifiez le log manuellement.")
    
    # Sauvegarder les données dans un format simple (CSV) pour le traçage
    output_csv = "simulation_data.csv"
    with open(output_csv, 'w') as f:
        # Écrire l'en-tête pour que numpy le reconnaisse
        f.write("Date,Duration_Hours\n") 
        for date, duration in parsed_data:
            f.write(f"{date},{duration:.4f}\n")
            
    print(f"[+] Données sauvegardées dans {output_csv} pour le traçage.")

