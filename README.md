# ☀️ Projet Jour - Simulateur de Durée du Jour (Géographie & Astronomie en C)

## 📜 Présentation du Projet

Bienvenue dans le **Simulateur de Durée du Jour**. Ce projet est une implémentation complète et rigoureuse en langage C, conçue pour modéliser et calculer la durée du jour (temps de lumière) sur une période donnée. Il sert de laboratoire pédagogique avancé, démontrant l'interaction complexe entre les concepts de géographie (latitude), de calendrier (dates), et d'astronomie (cycle solaire).

L'objectif n'est pas seulement de trouver un chiffre, mais de construire un système logiciel *robuste* qui gère :
1.  La lecture et l'interprétation de données textuelles imparfaites.
2.  La conversion entre différents systèmes de coordonnées (texte $\leftrightarrow$ numérique, calendaire $\leftrightarrow$ ordinal).
3.  L'application de modèles physiques complexes (Déclinaison Solaire).
4.  La gestion des erreurs en profondeur, du niveau de fichier au niveau de la logique métier.

---

## 🏛️ Architecture Modulaire : La Séparation des Préoccupations (SoC)

La force de ce projet réside dans sa modularité. Chaque fichier est responsable d'une seule tâche bien définie. Voici une description détaillée de chaque composant :

### ⚙️ Modules d'Utilitaires & Support
*   **`config.h`**: **Le Catalogue de Paramètres**. Définit les constantes fondamentales du système (ex: `MIN_YEAR`, `MAX_YEAR`, `MODE_SINUSOIDAL`). Il sert de référentiel unique pour toutes les limites de validation.
*   **`string_utils.c/h`**: **La Boîte à Outils de Chaînes**. Fournit des fonctions utilitaires génériques, principalement `strtrim()`, essentielle pour nettoyer les entrées de configuration en supprimant les espaces superflus.
*   **`logger.c/h`**: **Le Journaliste de Bord Configurable**. Ce module gère toute la sortie informative.
    *   **Mécanisme :** Il lit son niveau de verbosité depuis un fichier de configuration (`solar_duration.cfg` par défaut).
    *   **Fonctionnalité :** Il permet de basculer entre `DEBUG` (très verbeux, pour le débogage), `INFO` (flux de travail normal), `WARN`, `ERROR` et `CRITICAL`.
    *   **Robustesse :** L'initialisation est configurable (`logger_init(NULL)` pour le défaut).

### 📅 Modules Temporels et Géographiques
*   **`date.c/h`**: **Le Gestionnaire Temporel (Calendrier $\leftrightarrow$ Ordinal)**. Ce module est le pont entre le monde humain (Jour/Mois/Année) et le monde mathématique (Jours Ordinaux, 1 à 366).
    *   **Fonctionnalités Clés :**
        *   `is_leap_year()`: Détermine les années bissextiles.
        *   `date_to_ordinal()`: Convertit une date complète en son positionnement dans l'année.
        *   `is_date_valid()`: Valide si une date existe réellement (ex: 30 février).
        *   `advance_day()`: Permet de "passer au jour suivant" de manière fiable, gérant automatiquement les changements de mois et d'année.
*   **`geo.c/h`**: **Le Traducteur Géométrique**. Ce module est spécialisé dans la conversion des entrées textuelles complexes en valeurs numériques précises.
    *   **Fonctionnalité Critique :** `parse_latitude_string()` prend une chaîne comme `"47 deg 17 min 48 sec Nord"` et la transforme en un `double` standardisé (ex: `47.296667`). Il gère également l'inversion de signe pour le Sud.

### ⚛️ Modules Scientifiques et de Simulation
*   **`solar.c/h`**: **Le Moteur Physique Astronomique**. C'est le cœur mathématique du projet.
    *   **Déclinaison Solaire ($\delta$)**: Il implémente plusieurs modèles pour calculer l'angle d'inclinaison du Soleil par rapport à l'équateur terrestre pour un jour donné :
        *   **Sinusoïdal (Mode 1)**: Calibré, approximation cyclique simple (~±1.5° d'erreur).
        *   **Spencer (Mode 2)**: Calibré, modèle trigonométrique précis (~±0.0006 rad). **[FONCTIONNEL]**
        *   **Meeus (Mode 3)**: Modèle avancé. **[À implémenter]** (voir `IMPROVEMENTS.md` pour détails).
    *   **Durée du Jour**: Utilise la déclinaison et la latitude dans des formules trigonométriques (basées sur $\omega$, l'angle horaire) pour déterminer la durée du jour en heures.
*   **`simulation.c/h`**: **Le Moteur de Boucle**. Ce module est le chef d'orchestre des calculs. Il prend la configuration valide et itère jour après jour (en utilisant `advance_day` de `date.c`) pour exécuter `process_day` pour chaque date de début à date de fin.

### 👑 Le Contrôleur Principal
*   **`main.c`**: **L'Orchestrateur**. Il dirige l'intégralité du processus :
    1.  Initialisation du Logger.
    2.  Appel à `read_raw_data()` pour charger les chaînes brutes.
    3.  Appel à `parse_and_validate_config()` pour convertir les chaînes en types numériques et valider les bornes (Année, Latitude, Plage de Dates).
    4.  Appel à `is_config_fully_valid()` pour une vérification logique finale.
    5.  Si tout est bon, il lance `run_simulation()`.
    6.  Gère le nettoyage des ressources allouées (mémoire).

---

## 🚀 Guide d'Utilisation Opérationnel (Workflow)

### **Phase 1 : Configuration des Entrées**
1.  **Configuration des Données (`solar_duration.dat`)** :
    *   Définissez l'année (`annee`), les dates de début et de fin (`jour_debut`/`mois_debut`, etc.), la latitude (`latitude`), et le modèle solaire souhaité (`mode_solaire`).
    *   **⚠️ Attention :** La syntaxe doit être respectée. Le système est tolérant aux commentaires (`//` ou `#`) et aux espaces superflus.
2.  **Configuration du Log (`solar_duration.cfg`)** :
    *   Définissez le niveau de verbosité (`level = INFO`, `DEBUG`, etc.). Cela contrôle ce que vous verrez à l'écran ou dans le fichier de log.

### **Phase 2 : Exécution du Programme**

Le système est piloté via le `Makefile`.

| Commande `make` | Description | Sortie / Résultat |
| :--- | :--- | :--- |
| `make run` | **Exécution standard.** Compile puis exécute le programme. | Les résultats de chaque jour sont affichés directement dans la console. |
| `make run_log` | **Exécution journalisée (Recommandé).** Compile et exécute, redirigeant *toutes* les sorties (logs, erreurs) vers le fichier spécifié. | Un fichier **`solar_duration.log`** est créé, contenant l'historique complet du processus. |
| `make clean` | **Maintenance.** Supprime l'exécutable, les objets temporaires, et **efface le journal de simulation** (`solar_duration.log`). | Le système revient à un état initial propre. |

### **Phase 3 : Assurance Qualité (DevOps)**

Pour garantir la qualité du code avant de lancer une simulation critique, utilisez les outils intégrés :

*   **`make format`**: Uniformise le style de codage en appliquant les règles de `clang-format`.
*   **`make lint`**: Lance `clang-tidy`. **Ceci est une vérification de style avancé et de failles logiques potentielles.**
*   **`make cppcheck`**: Lance `cppcheck`. **Ceci est une vérification structurelle très poussée** des erreurs de mémoire, des boucles infinies, etc.

---

## 🐛 Gestion des Erreurs et Diagnostics

Le système est conçu pour ne jamais planter sans avertissement. Si une erreur survient, vous recevrez un code de retour d'erreur et un message détaillé.

**Types d'erreurs rencontrées et leur signification :**

| Code/Fonction | Type d'Erreur | Cause Typique | Action Requise |
| :--- | :--- | :--- | :--- |
| `READ_ERROR_IO_FILE_NOT_FOUND` | **I/O** | Le fichier de données (`.dat`) ou de configuration (`.cfg`) est manquant. | Vérifiez le chemin et le nom de fichier. |
| `READ_ERROR_DATA_MISSING` | **Configuration** | Le fichier a été lu, mais aucune clé (`annee`, `latitude`, etc.) n'a été trouvée. | Assurez-vous que tous les champs obligatoires sont présents dans `.dat`. |
| `READ_ERROR_DATA_CONVERSION` | **Format** | Une valeur est présente, mais n'est pas dans le format attendu (ex: `latitude = "abc"` au lieu de `"47 deg..."`). | Corrigez la syntaxe dans le fichier d'entrée. |
| `is_config_fully_valid` retourne `false` | **Logique Métier** | Les bornes physiques sont dépassées (ex: Année > 3000, Latitude > 90). | Ajustez les valeurs dans `.dat` pour qu'elles respectent les constantes de `config.h`. |
| `is_time_range_valid` retourne `false` | **Séquence Temporelle** | La date de début est postérieure à la date de fin. | Inverser les dates dans `.dat`. |

---

## 📊 État du Projet et Limitations Connues

### **État Actuel (2026-09-05)**

✅ **Fonctionnalités Complètes** :
- Lecture et validation de configuration
- Conversions calendaires (date ↔ ordinal)
- Parsing de coordonnées géographiques (degrés/minutes/secondes)
- Calcul de déclinaison solaire (modes Sinusoïdal & Spencer)
- Simulation jour par jour avec logging multi-niveaux
- Assurance qualité (linting, formatting, static analysis)

⏳ **Fonctionnalités Partielles/Incomplètes** :
- **Mode Meeus** (`mode_solaire = 3`) : Fonction stub retourne 0.0, non implémentée (voir `IMPROVEMENTS.md`)
- **Support multi-années** : Les dates doivent actuellement être sur la même année calendaire

🐛 **Bugs Connus** :
Voir le document `IMPROVEMENTS.md` pour liste détaillée. Les plus critiques :
1. Validation latitude accepte degrés > 180 (devrait être > 90)
2. Comptage de paramètres configuration cassé (ne détecte pas champs manquants)
3. Pointeur fonction déclinaison non vérifié (NULL check manquant)

⚠️ **Limitations par Conception** :
- Plage de dates : Même année calendaire (2026/08/21 → 2026/08/21, pas 2026/12 → 2027/01)
- Latitude : [-90°, 90°] strictement
- Année : [1900, 3000]
- Format latitude : Degrés/minutes/secondes uniquement (pas de degrés décimaux directs)

### **Roadmap de Correction**

Consultez `IMPROVEMENTS.md` pour :
- Liste complète des 24+ opportunités d'amélioration
- Priorisation (Haute/Moyenne/Basse)
- Effort estimé pour chaque correction
- Plan d'implémentation par phases

---

## 🤝 Contribution et Licence

**Licence :** Ce projet est distribué sous licence MIT. Les termes de cette licence sont rédigés dans le fichier `LICENSE` (si présent) ou sous-entendus par l'utilisation.

**Contribuer :** Nous encourageons vivement les contributions ! Avant de commencer :
1. Consultez `IMPROVEMENTS.md` pour connaître les priorités
2. Exécutez `make lint` et `make cppcheck` avant tout commit
3. Ajoutez des tests pour nouvelles fonctionnalités
4. Documentez les changements dans les commentaires de code

Priorités actuelles : Correctifs critiques (bugs #1-3), puis refactoring qualité.
