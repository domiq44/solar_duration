# 🧪 Protocole de Test Intégral du Projet Solar Duration

Ce protocole est conçu pour valider chaque couche de votre système, de la compilation du code C jusqu'à la génération finale des graphiques Python. Il doit être exécuté séquentiellement pour garantir une couverture complète des fonctionnalités.

## 🧭 Vue d'Ensemble du Workflow

Le test se divise en trois phases croisées :
1. **Phase 1 (Core) :** Valider la compilation et la logique C pure.
2. **Phase 2 (Standard) :** Valider le flux de travail de simulation normale (Log $\rightarrow$ CSV $\rightarrow$ Plot).
3. **Phase 3 (Intégration) :** Valider le scénario de test spécifique (Comparaison des 3 modèles).

---

## 🟢 Phase 1 : Validation du Cœur (Core Logic & Build)

L'objectif est de s'assurer que le compilateur, le *linker* et les outils d'analyse statique fonctionnent sans erreur.

### ⚙️ Commandes à Exécuter
```bash
make all
make test_unit
make format
make lint
make cppcheck
make clean
```

### ✅ Points de Vérification
*   **Compilation (`make all`):** L'exécutable `solar_duration` doit être présent.
*   **Unitaires (`make test_unit`):** Tous les tests doivent réussir (`PASS`). Si un test échoue, le problème est dans `date.c`, `geo.c`, ou `solar.c` avant même d'atteindre Python.
*   **Analyse Statique (`make lint`, `make cppcheck`):** Les rapports doivent être générés. Ces commandes ne doivent pas planter le processus.
*   **Nettoyage (`make clean`):** Tous les fichiers générés doivent disparaître proprement.

---

## 🟡 Phase 2 : Validation du Cycle de Simulation Standard (Flux de Travail)

Cette phase valide le chemin critique : **Configuration $\rightarrow$ Simulation $\rightarrow$ Journalisation $\rightarrow$ Visualisation.**

### 🚀 Commandes à Exécuter
```bash
make run
make run_log
make run_duration_plot
```

### ✅ Points de Vérification
1.  **Exécution Standard (`make run`):** Vérifiez que les résultats de la simulation sont affichés correctement en console (format H:M:S).
2.  **Journalisation (`make run_log`):** Vérifiez que `solar_duration.log` est créé. Ouvrez-le et confirmez qu'il contient bien les logs de simulation et qu'il est formaté correctement avec le timestamp.
3.  **Analyse Finale (`make run_duration_plot`):**
    *   Vérifiez que le `simulation_data.csv` est créé avec les bonnes dates et les durées en heures (format décimal).
    *   Vérifiez que `solar_duration.png` est généré. **Inspection visuelle :** La courbe doit montrer une variation saisonnière logique (pic en été, creux en hiver).

---

## 🔬 Phase 3 : Validation de l'Intégration Spécifique (Tests Solaire)

Cette phase valide le scénario de test spécifique, qui est plus complexe car il lance les *trois* modèles de déclinaison.

### 🔬 Commandes à Exécuter
```bash
make test_solar
make run_solar_test_plot
```

### ✅ Points de Vérification
1.  **Tests Solaires (`make test_solar`):** Confirmez que le runner C s'exécute et génère le fichier de test (`solar_declination_test.csv`).
2.  **Workflow de Comparaison (`make run_solar_test_plot`):**
    *   Vérifiez que le processus s'exécute jusqu'à la fin.
    *   Vérifiez que `solar_declination_comparison_plot.png` est généré. **Inspection visuelle :** Les trois courbes (Sinusoidal, Spencer, Meeus) doivent être **extrêmement proches** l'une de l'autre, démontrant la cohérence mathématique entre les trois implémentations.

---

## 🛑 En Cas d'Échec (Diagnostic)

Si une étape échoue, utilisez cette hiérarchie pour localiser le problème :

1.  **Échec en Phase 1 (Build/Unitaires) :** Le problème est dans le **code C** lui-même (syntaxe, logique de base). $\rightarrow$ Corriger dans `src/`.
2.  **Échec en Phase 2 (Run/Log) :** Le problème est dans la **lecture/parsing des fichiers** ou dans la **logique de boucle C**. $\rightarrow$ Vérifier `config_reader.c`, `simulation.c`, et `logger.c`.
3.  **Échec en Phase 2/3 (Plotting) :** Le problème est dans **Python** ou dans la **communication entre C et Python**. $\rightarrow$ Vérifier `log_parser.py` et `data_plotter.py`.

---

## 🤖 Automatisation Complète (Le Niveau Expert)

Pour passer de la simple *exécution* à l'*automatisation complète*, vous devez ajouter des **assertions de contenu** :

*   **Validation CSV :** Ajoutez des scripts Python de test qui lisent `simulation_data.csv` et vérifient que le nombre de lignes est correct et que la moyenne des durées est dans une plage acceptable.
*   **Validation PNG :** Pour une couverture maximale, des scripts d'analyse d'images (ex: avec Pillow) devraient confirmer la présence de marqueurs visuels attendus dans le graphique.

**Recommandation Finale :** Intégrez une cible `make check_all` dans votre `Makefile` qui orchestre toutes les commandes ci-dessus, vous donnant un seul point d'entrée pour valider l'intégralité du système.
