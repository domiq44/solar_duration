# --- Variables de Configuration du Projet ---
# Nom du fichier de données d'entrée principal
DATA_FILE = solar_duration.dat
# Nom du fichier de configuration du logger
LOG_CONFIG_FILE = solar_duration.cfg
# Nom du fichier de sortie du journal (pour 'make run_log')
LOG_OUTPUT_FILE = solar_duration.log

# --- Définitions de base ---
TARGET = solar_duration
SRCS = src/config_reader.c src/config_validator.c src/date.c src/geo.c src/logger.c src/main.c src/simulation.c src/solar.c src/string_utils.c
TEST_TARGET = solar_duration_tests
TEST_SRCS = src/config_reader.c src/config_validator.c src/date.c src/geo.c src/logger.c src/solar.c src/string_utils.c tests/tests.c
HDRS = include/config.h include/config_reader.h include/config_validator.h include/data_types.h include/date.h include/geo.h include/logger.h include/simulation.h include/solar.h include/string_utils.h
# Définir l'ensemble des fichiers à formater
ALL_SOURCES = $(SRCS) $(HDRS)

CC = gcc
CFLAGS = -Wall -O2 -pedantic -Iinclude
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

$(TEST_TARGET): $(TEST_SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRCS) $(LDFLAGS)

# --- Règles d'exécution (Utilisation des variables) ---

run: $(TARGET)
	./$(TARGET)

run_log: $(TARGET)
	./$(TARGET) > $(LOG_OUTPUT_FILE) 2>&1

test: $(TEST_TARGET)
	./$(TEST_TARGET)

# --- Règles de maintenance (Utilisation des variables) ---

clean:
	# Nettoie l'exécutable, les fichiers objets, les rapports d'analyse, ET le journal de simulation
	rm -f $(TARGET) $(TEST_TARGET) $(TARGET).o cppcheck_report.txt lint_report.txt $(LOG_OUTPUT_FILE)

format:
	clang-format -i $(ALL_SOURCES)

lint:
	@which clang-tidy > /dev/null || { echo "Erreur : clang-tidy non installé. Veuillez l'installer."; exit 1; }
	clang-tidy $(LINT_FLAGS) $(SRCS) -- -Iinclude > lint_report.txt

cppcheck:
	@which cppcheck > /dev/null || { echo "Erreur : cppcheck non installé. Veuillez l'installer..."; exit 1; }
	cppcheck $(CPPCHECK_FLAGS) -Iinclude $(SRCS) > cppcheck_report.txt

help:
	@echo "============================================================"
	@echo "         🛠️ GUIDE D'UTILISATION DU MAKEFILE 'solar_duration'"
	@echo "============================================================"
	@echo ""
	@echo "🚀 EXÉCUTION :"
	@echo "  make            : Compile le programme (crée l'exécutable '$(TARGET)')."
	@echo "  make run        : Exécute le programme et affiche les résultats en CONSOLE."
	@echo "  make run_log    : Exécute le programme et redirige TOUT le journal vers '$(LOG_OUTPUT_FILE)'."
	@echo "  make test       : Compile puis exécute les tests unitaires C."
	@echo ""
	@echo "🧹 MAINTENANCE & QUALITÉ :"
	@echo "  make clean      : Supprime l'exécutable, les fichiers objets, les rapports d'analyse, ET le journal de simulation ('$(LOG_OUTPUT_FILE)')."
	@echo "  make format     : Reformate le code source selon les standards (clang-format)."
	@echo "  make lint       : Analyse statique avec clang-tidy (Recommandé pour la qualité de code)."
	@echo "  make cppcheck   : Analyse statique avec cppcheck (Vérification approfondie des structures)."
	@echo ""
	@echo "============================================================"
