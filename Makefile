# ==========================================================================
# CONFIGURATION GLOBALE DU PROJET SOLAR_DURATION
# ==========================================================================

# --- ⚙️ Définitions des Sources et Headers ---
SRC_C_FILES := src/config_reader.c src/config_validator.c src/date.c src/geo.c src/logger.c src/main.c src/simulation.c src/solar.c src/string_utils.c
TEST_SRC_FILES := src/config_reader.c src/config_validator.c src/date.c src/geo.c src/logger.c src/solar.c src/string_utils.c tests/tests.c
TEST_SOLAR_SRC_FILES := src/config_reader.c src/config_validator.c src/date.c src/geo.c src/logger.c src/solar.c src/string_utils.c tests/solar_test_runner.c

# --- 📚 Dépendances d'en-tête ---
HEADER_FILES := include/config.h include/config_reader.h include/config_validator.h include/data_types.h include/date.h include/geo.h include/logger.h include/simulation.h include/solar.h include/string_utils.h

# --- 🧱 Artefacts et Entrées/Sorties ---
TARGET_MAIN_EXE := solar_duration
TEST_UNIT_EXE := solar_duration_tests
TEST_SOLAR_EXE := solar_test_runner

DATA_INPUT_FILE := solar_duration.dat
LOG_CONFIG_FILE := solar_duration.cfg
LOG_RUNTIME_FILE := solar_duration.log

# --- 🗄️ Fichiers de Sortie/Analyse ---
CSV_OUTPUT_DATA := solar_duration.csv
PLOT_DURATION_OUTPUT := solar_duration.png
CSV_TEST_DATA := solar_declination.csv
PLOT_SOLAR_OUTPUT := solar_declination.png

# --- 🛠️ Outils et Compilateurs ---
CC := gcc
CFLAGS := -Wall -O2 -pedantic -Wno-pedantic -Iinclude
LDFLAGS := -lm
PYTHON_CMD := python3

# --- 🛡️ Configurations des outils d'analyse statique ---
CPPCHECK_ARGS := \
	--quiet --enable=all --error-exitcode=1 \
	--inline-suppr \
	--suppress=missingIncludeSystem \
	--suppress=unmatchedSuppression \
	--suppress=unusedFunction \
	--check-level=exhaustive
LINT_ARGS := -checks='bugprone-*,-readability-*,-modernize-*'

# ==========================================================================
# CIBLES DE COMPILATION (BUILD)
# ==========================================================================

.PHONY: all clean format lint cppcheck help

all: $(TARGET_MAIN_EXE)

# 🎯 CIBLE PRINCIPALE : Compilation de l'exécutable principal
$(TARGET_MAIN_EXE): $(SRC_C_FILES) $(HEADER_FILES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# 🎯 CIBLE TESTS UNITAIRES
$(TEST_UNIT_EXE): $(TEST_SRC_FILES) $(HEADER_FILES)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC_FILES) $(LDFLAGS)

# 🎯 CIBLE TESTS SOLAIRES
$(TEST_SOLAR_EXE): $(TEST_SOLAR_SRC_FILES) $(HEADER_FILES)
	$(CC) $(CFLAGS) -g -o $@ $(TEST_SOLAR_SRC_FILES) $(LDFLAGS)


# ==========================================================================
# CIBLES D'EXÉCUTION ET DE SIMULATION (RUN/TEST)
# ==========================================================================

# 🚀 Exécution simple en console
run: $(TARGET_MAIN_EXE)
	@echo "--- 🚀 Exécution simple de $(TARGET_MAIN_EXE) ---"
	./$(TARGET_MAIN_EXE)

# 💾 Exécution complète avec journalisation (La plus recommandée)
run_log: $(TARGET_MAIN_EXE)
	@echo "--- 💾 Exécution journalisée vers $(LOG_RUNTIME_FILE) ---"
	./$(TARGET_MAIN_EXE) > $(LOG_RUNTIME_FILE) 2>&1

# 🧪 Exécution des tests unitaires C
test_unit: $(TEST_UNIT_EXE)
	@echo "--- 🧪 Exécution des Tests Unitaires C ($(TEST_UNIT_EXE)) ---"
	./$(TEST_UNIT_EXE)

# 🔬 Exécution des tests d'intégration solaire
test_solar: $(TEST_SOLAR_EXE)
	@echo "--- 🔬 Exécution des Tests d'Intégration Solaires ($(TEST_SOLAR_EXE)) ---"
	./$(TEST_SOLAR_EXE)

# 📈 Workflow complet pour le tracé de la durée journalière (VERSION FINALE)
run_duration_plot: $(TARGET_MAIN_EXE)
	@echo "========================================================"
	@echo "🚀 DÉBUT DE L'ANALYSE DE DONNÉES (DURÉE JOURNALIÈRE)"
	@echo "========================================================"
	
	# 1. Exécuter la simulation pour garantir que le log est créé
	@echo "-> Étape 1/3 : Exécution de la simulation pour générer le log..."
	./$(TARGET_MAIN_EXE) > $(LOG_RUNTIME_FILE) 2>&1
	
	# 2. Parser le log -> CRÉE solar_duration.csv
	@echo "-> Étape 2/3 : Extraction des données du log..."
	$(PYTHON_CMD) tools/log_parser.py
	
	# 3. Tracer le graphique -> Utilise solar_duration.csv (Ceci crée le PNG)
	@echo "-> Étape 3/3 : Génération du fichier PNG..."
	$(PYTHON_CMD) tools/data_plotter.py

	@echo "========================================================"
	@echo "🎉 ANALYSE TERMINÉE. Vérifiez '$(PLOT_DURATION_OUTPUT)'."
	@echo "========================================================"

# 🔬 Workflow complet pour le tracé de la comparaison des modèles (VERSION ULTIME CORRIGÉE)
run_solar_test_plot: $(TEST_SOLAR_EXE)
	@echo "=========================================================="
	@echo "🔬 DÉBUT DU TEST D'INTÉGRATION SOLAIRE (COMPARAISON DES MODÈLES)"
	@echo "=========================================================="
	
	# 1. Lancer le runner C pour générer le fichier de test (Le prérequis)
	@echo "-> Étape 1/2 : Lancement du test C pour générer $(CSV_TEST_DATA)..."
	@export SOLAR_DEBUG=False
	./$(TEST_SOLAR_EXE) > $(CSV_TEST_DATA) 2>&1
	
	# 2. Tracer le graphique de comparaison (Ceci utilise CSV_TEST_DATA, pas le log)
	@echo "-> Étape 2/2 : Génération du fichier PNG..."
	$(PYTHON_CMD) tools/solar_plotter.py

	@echo "=========================================================="
	@echo "🎉 TEST SOLAIRE TERMINÉ. Vérifiez '$(PLOT_SOLAR_OUTPUT)'."
	@echo "=========================================================="

# ==========================================================================
# CIBLES DE MAINTENANCE ET QUALITÉ (DEVOPs)
# ==========================================================================

clean:
	@echo "🧹 Nettoyage : Suppression des exécutables, objets, logs et rapports..."
	rm -f $(TARGET_MAIN_EXE) $(TEST_UNIT_EXE) $(TEST_SOLAR_EXE) *.o *.a *.so *.dll *.exe \
	$(LOG_RUNTIME_FILE) $(CSV_OUTPUT_DATA) $(PLOT_DURATION_OUTPUT) $(CSV_TEST_DATA) $(PLOT_SOLAR_OUTPUT) \
	cppcheck_report.txt lint_report.txt

format:
	@echo "🎨 Formatage du code avec clang-format..."
	clang-format -i $(SRC_C_FILES) $(HEADER_FILES)

lint:
	@which clang-tidy > /dev/null || { echo "Erreur: clang-tidy non installé. Veuillez l'installer."; exit 1; }
	echo "🔍 Lancement de l'analyse statique avec clang-tidy..."
	clang-tidy $(LINT_ARGS) $(SRC_C_FILES) -- -Iinclude > cppcheck_report.txt # <-- J'ai renommé le fichier de sortie pour ne pas le confondre avec cppcheck
	
cppcheck:
	@which cppcheck > /dev/null || { echo "Erreur: cppcheck non installé. Veuillez l'installer..."; exit 1; }
	echo "🔬 Lancement de l'analyse structurelle avec cppcheck..."
	cppcheck $(CPPCHECK_ARGS) -Iinclude $(SRC_C_FILES) > cppcheck_report.txt

help:
	@echo "============================================================"
	@echo "         🛠️ GUIDE D'UTILISATION DU MAKEFILE 'solar_duration'"
	@echo "============================================================"
	@echo ""
	@echo "🚀 EXÉCUTION & TEST :"
	@echo "  make all / make $(TARGET_MAIN_EXE) : Compile tous les exécutables."
	@echo "  make run                   : Exécute la simulation en console (affichage immédiat)."
	@echo "  make run_log               : Exécute la simulation et sauvegarde tout dans $(LOG_RUNTIME_FILE)."
	@echo "  make test_unit             : Exécute les tests unitaires C."
	@echo "  make test_solar            : Exécute les tests d'intégration solaire."
	@echo ""
	@echo "📊 ANALYSE ET TRACAGE (Workflow complet) :"
	@echo "  make run_duration_plot     : Simule, parse le log, et génère '$(PLOT_DURATION_OUTPUT)'."
	@echo "  make run_solar_test_plot   : Exécute le test solaire, parse le log, et génère '$(PLOT_SOLAR_OUTPUT)'."
	@echo ""
	@echo "🧹 MAINTENANCE & QUALITÉ :"
	@echo "  make clean                 : Supprime tous les fichiers générés (exécutables, logs, rapports)."
	@echo "  make format                : Reformate le code source avec clang-format."
	@echo "  make lint                  : Analyse statique avancée avec clang-tidy (Résultat: cppcheck_report.txt)."
	@echo "  make cppcheck              : Analyse structurelle approfondie avec cppcheck (Résultat: cppcheck_report.txt)."
	@echo "============================================================"
