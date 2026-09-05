# --- Variables de Configuration du Projet ---
# Nom du fichier de données d'entrée principal
DATA_FILE = solar_duration.dat
# Nom du fichier de configuration du logger
LOG_CONFIG_FILE = solar_duration.cfg
# Nom du fichier de sortie du journal (pour 'make run_log')
LOG_OUTPUT_FILE = solar_duration.log

# --- Définitions de base ---
TARGET = solar_duration
SRCS = config_reader.c config_validator.c date.c geo.c logger.c main.c simulation.c solar.c string_utils.c
HDRS = config.h config_reader.h config_validator.h data_types.h date.h geo.h logger.h simulation.h solar.h string_utils.h
# Définir l'ensemble des fichiers à formater
ALL_SOURCES = $(SRCS) $(HDRS)

CC = gcc
CFLAGS = -Wall -O2 -pedantic
LDFLAGS = -lm

.PHONY: run run_log test clean format lint cppcheck help

# --- Configuration des outils d'analyse statique ---
# ... (Les flags restent inchangés) ...
CPPCHECK_FLAGS = \
	--quiet --enable=all --error-exitcode=1 \
	--inline-suppr \
	--suppress=missingIncludeSystem \
	--suppress=unmatchedSuppression \
	--suppress=unusedFunction \
	--check-level=exhaustive

LINT_FLAGS = -checks='bugprone-*,-readability-*,-modernize-*'

# --- Règle principale : Compilation ---
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# --- Règles d'exécution (Utilisation des variables) ---

run: $(TARGET)
	./$(TARGET)

run_log: $(TARGET)
	./$(TARGET) > $(LOG_OUTPUT_FILE) 2>&1

test: $(TARGET)
	./run_tests.sh
	./test_medium_fixes.sh

# --- Règles de maintenance (Utilisation des variables) ---

clean:
	# Nettoie l'exécutable, les fichiers objets, les rapports d'analyse, ET le journal de simulation
	rm -f $(TARGET) $(TARGET).o cppcheck_report.txt lint_report.txt $(LOG_OUTPUT_FILE)

format:
	clang-format -i $(ALL_SOURCES)

lint:
	@which clang-tidy > /dev/null || { echo "Erreur : clang-tidy non installé. Veuillez l'installer."; exit 1; }
	clang-tidy $(LINT_FLAGS) $(SRCS) -- -I. > lint_report.txt

cppcheck:
	@which cppcheck > /dev/null || { echo "Erreur : cppcheck non installé. Veuillez l'installer..."; exit 1; }
	cppcheck $(CPPCHECK_FLAGS) $(SRCS) > cppcheck_report.txt

help:
	@echo "============================================================"
	@echo "         🛠️ GUIDE D'UTILISATION DU MAKEFILE 'solar_duration'"
	@echo "============================================================"
	@echo ""
	@echo "🚀 EXÉCUTION :"
	@echo "  make            : Compile le programme (crée l'exécutable '$(TARGET)')."
	@echo "  make run        : Exécute le programme et affiche les résultats en CONSOLE."
	@echo "  make run_log    : Exécute le programme et redirige TOUT le journal vers '$(LOG_OUTPUT_FILE)'."
	@echo "  make test       : Compile puis exécute toutes les suites de tests."
	@echo ""
	@echo "🧹 MAINTENANCE & QUALITÉ :"
	@echo "  make clean      : Supprime l'exécutable, les fichiers objets, les rapports d'analyse, ET le journal de simulation ('$(LOG_OUTPUT_FILE)')."
	@echo "  make format     : Reformate le code source selon les standards (clang-format)."
	@echo "  make lint       : Analyse statique avec clang-tidy (Recommandé pour la qualité de code)."
	@echo "  make cppcheck   : Analyse statique avec cppcheck (Vérification approfondie des structures)."
	@echo ""
	@echo "============================================================"
