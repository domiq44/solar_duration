# ==========================================================================
# CONFIGURATION GLOBALE DU PROJET SOLAR_DURATION
# ==========================================================================

# --- Sources et en-têtes ---
CORE_SOURCES := \
	src/config_reader.c \
	src/config_validator.c \
	src/date.c \
	src/geo.c \
	src/logger.c \
	src/simulation.c \
	src/solar.c \
	src/string_utils.c \
	src/web_api.c
APP_SOURCES := $(CORE_SOURCES) src/main.c

TEST_FRAMEWORK_SOURCES := \
	tests/support/test_assertions.c \
	tests/support/test_reporter.c \
	tests/support/test_runner.c
TEST_FRAMEWORK_HEADERS := \
	tests/support/test_assertions.h \
	tests/support/test_reporter.h \
	tests/support/test_runner.h
UNIT_TEST_SOURCES := $(CORE_SOURCES) $(TEST_FRAMEWORK_SOURCES) \
	tests/support/test_file_utils.c
UNIT_TEST_HEADERS := $(TEST_FRAMEWORK_HEADERS) tests/support/test_file_utils.h
SOLAR_DATA_GENERATOR_SOURCES := $(CORE_SOURCES) tools/solar_data_generator.c
LINT_SOURCES := $(APP_SOURCES) tools/solar_data_generator.c

# --- Répertoires et exécutables ---
OUTPUT_DIR := output
BUILD_DIR := build
TEST_BUILD_DIR := $(BUILD_DIR)/tests
COVERAGE_DIR := $(BUILD_DIR)/coverage
COVERAGE_OBJECT_DIR := $(COVERAGE_DIR)/objects
COVERAGE_REPORT_DIR := coverage
COVERAGE_HTML_REPORT := $(COVERAGE_REPORT_DIR)/index.html
COVERAGE_TEST_BUILD_DIR := $(COVERAGE_DIR)/tests
UNIT_TEST_MODULES := \
	config_reader \
	config_validator \
	date \
	geo \
	logger \
	simulation \
	solar \
	string_utils
UNIT_TEST_EXECUTABLES := $(addprefix $(TEST_BUILD_DIR)/solar_duration_tests_,\
	$(UNIT_TEST_MODULES))
INTEGRATION_TEST_EXECUTABLE := $(TEST_BUILD_DIR)/solar_duration_integration_tests
SOLAR_DATA_GENERATOR := $(TEST_BUILD_DIR)/solar_data_generator
APP_EXECUTABLE := $(BUILD_DIR)/solar_duration
COVERAGE_CFLAGS = $(filter-out -O2,$(CFLAGS)) -O0 -g --coverage
COVERAGE_CORE_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/,$(CORE_SOURCES:.c=.o))
COVERAGE_FRAMEWORK_OBJECTS := $(addprefix $(COVERAGE_OBJECT_DIR)/,$(TEST_FRAMEWORK_SOURCES:.c=.o))
COVERAGE_FILE_UTILS_OBJECT := $(COVERAGE_OBJECT_DIR)/tests/support/test_file_utils.o
COVERAGE_TEST_EXECUTABLES := $(addprefix $(COVERAGE_TEST_BUILD_DIR)/solar_duration_tests_,\
	$(UNIT_TEST_MODULES))
COVERAGE_APP_EXECUTABLE := $(COVERAGE_DIR)/solar_duration
COVERAGE_INTEGRATION_TEST := $(COVERAGE_TEST_BUILD_DIR)/solar_duration_integration_tests
COVERAGE_SOLAR_DATA_GENERATOR := $(COVERAGE_DIR)/solar_data_generator
COVERAGE_SOURCES := $(APP_SOURCES) tools/solar_data_generator.c
COVERAGE_REPORT_FILES := $(addsuffix .gcov,$(notdir $(COVERAGE_SOURCES)))

# --- En-têtes publics ---
PUBLIC_HEADERS := \
	include/config.h \
	include/config_reader.h \
	include/config_validator.h \
	include/data_types.h \
	include/date.h \
	include/geo.h \
	include/logger.h \
	include/simulation.h \
	include/solar.h \
	include/string_utils.h \
	include/web_api.h

# --- Fichiers d'entrée et sorties ---
LOG_RUNTIME_FILE := $(OUTPUT_DIR)/solar_duration.log
CSV_OUTPUT_DATA := $(OUTPUT_DIR)/solar_duration.csv
PLOT_DURATION_OUTPUT := $(OUTPUT_DIR)/solar_duration.png
PLOT_LATITUDES_OUTPUT := $(OUTPUT_DIR)/solar_latitudes.png
PLOT_DURATION_HEATMAP_OUTPUT := $(OUTPUT_DIR)/solar_duration_heatmap.png
SOLAR_MODEL_CSV := $(OUTPUT_DIR)/solar_model_comparison.csv
SOLAR_MODEL_PLOT := $(OUTPUT_DIR)/solar_model_comparison.png
SOLAR_DATA_LOG := $(OUTPUT_DIR)/solar_data_generator.log

# --- Compilateur et outils ---
CC := gcc
CFLAGS := -std=c11 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -Wall -Wextra -Wpedantic -O2 -Iinclude
LDFLAGS := -lm
PYTHON_CMD := python3
WEB_PORT ?= 8000
NODE_CMD := node
GCOVR := gcovr
EMCC := emcc
WASM_MODULE := web/solar_core.mjs
WASM_SOURCES := src/date.c src/solar.c src/web_api.c

# --- Analyse statique ---
CPPCHECK_ARGS := \
	--quiet --enable=all --error-exitcode=1 \
	--suppress=missingIncludeSystem \
	--check-level=exhaustive
CPPCHECK_SOURCES := $(sort $(APP_SOURCES) $(wildcard tests/test_*.c) \
	$(wildcard tests/support/*.c) tools/solar_data_generator.c)

# ==========================================================================
# CIBLES DE COMPILATION
# ==========================================================================

.PHONY: all run run_log run_web test test_unit test_integration coverage web_wasm test_web_wasm
.PHONY: generate_solar_data plot_duration plot_solar_models plot_latitudes
.PHONY: plot_duration_heatmap plot_all
.PHONY: clean format lint cppcheck help

all: $(APP_EXECUTABLE)

# Application principale
$(APP_EXECUTABLE): $(APP_SOURCES) $(PUBLIC_HEADERS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -o $@ $(APP_SOURCES) $(LDFLAGS)

# Suites unitaires
$(UNIT_TEST_EXECUTABLES): $(TEST_BUILD_DIR)/solar_duration_tests_%: \
		$(UNIT_TEST_SOURCES) $(UNIT_TEST_HEADERS) $(PUBLIC_HEADERS) tests/test_%.c
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -Itests/support -o $@ $(UNIT_TEST_SOURCES) \
		tests/test_$*.c $(LDFLAGS)

# Test d'intégration de l'application
$(INTEGRATION_TEST_EXECUTABLE): tests/test_application.c \
		$(TEST_FRAMEWORK_SOURCES) $(TEST_FRAMEWORK_HEADERS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -Itests/support -o $@ tests/test_application.c \
		$(TEST_FRAMEWORK_SOURCES) $(LDFLAGS)

# Générateur de données solaires
$(SOLAR_DATA_GENERATOR): $(SOLAR_DATA_GENERATOR_SOURCES) $(PUBLIC_HEADERS)
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -g -o $@ $(SOLAR_DATA_GENERATOR_SOURCES) $(LDFLAGS)


# ==========================================================================
# CIBLES D'EXÉCUTION ET DE TEST
# ==========================================================================

# 🚀 Exécution simple en console
run: $(APP_EXECUTABLE)
	@echo "--- Exécution de $(APP_EXECUTABLE) ---"
	@./$(APP_EXECUTABLE)

# 💾 Exécution complète avec journalisation (La plus recommandée)
run_log: $(APP_EXECUTABLE)
	@mkdir -p $(OUTPUT_DIR)
	@echo "--- Exécution journalisée vers $(LOG_RUNTIME_FILE) ---"
	@./$(APP_EXECUTABLE) > $(LOG_RUNTIME_FILE) 2>&1

# 🧪 Exécution des tests unitaires C
test_unit: $(UNIT_TEST_EXECUTABLES)
	@echo "--- Exécution des suites unitaires C ---"
	@set -e; for test_executable in $(UNIT_TEST_EXECUTABLES); do \
		./$$test_executable; \
	done

test_integration: $(APP_EXECUTABLE) $(INTEGRATION_TEST_EXECUTABLE)
	@./$(INTEGRATION_TEST_EXECUTABLE) ./$(APP_EXECUTABLE)

test: test_unit test_integration

run_web: web_wasm
	@echo "Serveur web disponible sur http://localhost:$(WEB_PORT) (Ctrl+C pour arrêter)"
	@$(PYTHON_CMD) -m http.server $(WEB_PORT) --directory web

$(COVERAGE_OBJECT_DIR)/%.o: %.c $(PUBLIC_HEADERS) $(UNIT_TEST_HEADERS)
	@mkdir -p $(@D)
	@$(CC) $(COVERAGE_CFLAGS) -Itests/support -c -o $@ $<

$(COVERAGE_TEST_EXECUTABLES): $(COVERAGE_TEST_BUILD_DIR)/solar_duration_tests_%: \
		$(COVERAGE_CORE_OBJECTS) $(COVERAGE_FRAMEWORK_OBJECTS) \
		$(COVERAGE_FILE_UTILS_OBJECT) $(COVERAGE_OBJECT_DIR)/tests/test_%.o
	@mkdir -p $(@D)
	@$(CC) --coverage -o $@ $^ $(LDFLAGS)

$(COVERAGE_APP_EXECUTABLE): $(COVERAGE_CORE_OBJECTS) \
		$(COVERAGE_OBJECT_DIR)/src/main.o
	@mkdir -p $(@D)
	@$(CC) --coverage -o $@ $^ $(LDFLAGS)

$(COVERAGE_INTEGRATION_TEST): \
		$(COVERAGE_OBJECT_DIR)/tests/test_application.o \
		$(COVERAGE_FRAMEWORK_OBJECTS)
	@mkdir -p $(@D)
	@$(CC) --coverage -o $@ $^ $(LDFLAGS)

$(COVERAGE_SOLAR_DATA_GENERATOR): $(COVERAGE_CORE_OBJECTS) \
		$(COVERAGE_OBJECT_DIR)/tools/solar_data_generator.o
	@mkdir -p $(@D)
	@$(CC) --coverage -o $@ $^ $(LDFLAGS)

coverage: $(COVERAGE_TEST_EXECUTABLES) $(COVERAGE_APP_EXECUTABLE) \
		$(COVERAGE_INTEGRATION_TEST) $(COVERAGE_SOLAR_DATA_GENERATOR)
	@command -v gcov >/dev/null || { echo "Erreur: gcov est requis (fourni avec GCC)."; exit 1; }
	@command -v $(GCOVR) >/dev/null || { echo "Erreur: gcovr est requis pour générer le rapport HTML."; exit 1; }
	@find $(COVERAGE_OBJECT_DIR) -name '*.gcda' -delete
	@mkdir -p $(COVERAGE_REPORT_DIR)
	@rm -f $(COVERAGE_REPORT_DIR)/*.gcov $(COVERAGE_REPORT_DIR)/*.html \
		$(COVERAGE_REPORT_FILES)
	@set -e; for test_executable in $(COVERAGE_TEST_EXECUTABLES); do \
		log_file=$(COVERAGE_DIR)/$$(basename $$test_executable).log; \
		./$$test_executable > $$log_file 2>&1 || { cat $$log_file; exit 1; }; \
	done
	@log_file=$(COVERAGE_DIR)/integration.log; \
		./$(COVERAGE_INTEGRATION_TEST) ./$(COVERAGE_APP_EXECUTABLE) \
		> $$log_file 2>&1 || { cat $$log_file; exit 1; }
	@mkdir -p $(COVERAGE_DIR)/runtime/config $(COVERAGE_DIR)/runtime/output
	@cp config/solar_duration.dat config/solar_data_generator.cfg \
		$(COVERAGE_DIR)/runtime/config/
	@log_file=$(COVERAGE_DIR)/solar_data_generator.log; \
		(cd $(COVERAGE_DIR)/runtime && ../solar_data_generator) \
		> $$log_file 2>&1 || { cat $$log_file; exit 1; }
	@echo "--- Couverture des sources src/ et tools/ (rapports .gcov dans $(COVERAGE_REPORT_DIR)) ---"
	@set -e; for source in $(COVERAGE_SOURCES); do \
		object=$(COVERAGE_OBJECT_DIR)/$${source%.c}.gcno; \
		gcov -b -c -o $$object $$source; \
	done
	@mv $(COVERAGE_REPORT_FILES) $(COVERAGE_REPORT_DIR)/
	@$(GCOVR) --root . --object-directory $(COVERAGE_OBJECT_DIR) \
		--filter 'src/.*\.c$$' --filter 'tools/.*\.c$$' \
		--html-details $(COVERAGE_HTML_REPORT) --print-summary
	@echo "Rapport détaillé : $(COVERAGE_HTML_REPORT)"

$(WASM_MODULE): $(WASM_SOURCES) include/config.h include/date.h include/solar.h include/web_api.h
	@$(EMCC) -std=c11 -O2 -Iinclude $(WASM_SOURCES) -lm \
		-sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web,node \
		-sALLOW_MEMORY_GROWTH=1 -o $@

web_wasm: $(WASM_MODULE)

test_web_wasm: $(WASM_MODULE)
	@$(NODE_CMD) tests/test_web_wasm.mjs

# 🔬 Génération des données de comparaison solaire
generate_solar_data: $(SOLAR_DATA_GENERATOR)
	@mkdir -p $(OUTPUT_DIR)
	@echo "--- Génération des données solaires ($(SOLAR_DATA_GENERATOR)) ---"
	@./$(SOLAR_DATA_GENERATOR) > $(SOLAR_DATA_LOG) 2>&1 || { cat $(SOLAR_DATA_LOG); exit 1; }

# 📈 Génération du graphique de durée journalière
plot_duration: $(APP_EXECUTABLE)
	@mkdir -p $(OUTPUT_DIR)
	@echo "========================================================"
	@echo "🚀 DÉBUT DE L'ANALYSE DE DONNÉES (DURÉE JOURNALIÈRE)"
	@echo "========================================================"
	
	@# 1. Générer directement le CSV depuis les résultats de simulation.
	@echo "-> Étape 1/2 : Simulation et export vers $(CSV_OUTPUT_DATA)..."
	@./$(APP_EXECUTABLE) --csv $(CSV_OUTPUT_DATA) > $(LOG_RUNTIME_FILE) 2>&1
	
	@# 2. Tracer le graphique à partir du CSV.
	@echo "-> Étape 2/2 : Génération du fichier PNG..."
	@$(PYTHON_CMD) tools/duration_plotter.py

	@echo "========================================================"
	@echo "🎉 ANALYSE TERMINÉE. Vérifiez '$(PLOT_DURATION_OUTPUT)'."
	@echo "========================================================"

# 🔬 Génération du graphique de comparaison des modèles
plot_solar_models: generate_solar_data
	@mkdir -p $(OUTPUT_DIR)
	@echo "=========================================================="
	@echo "🔬 GÉNÉRATION DU GRAPHIQUE DE COMPARAISON DES MODÈLES"
	@echo "=========================================================="
	@echo "-> Génération du fichier PNG..."
	@SOLAR_DEBUG=False $(PYTHON_CMD) tools/solar_plotter.py

	@echo "=========================================================="
	@echo "🎉 COMPARAISON SOLAIRE TERMINÉE. Vérifiez '$(SOLAR_MODEL_PLOT)'."
	@echo "=========================================================="

# 🌍 Comparaison de la durée du jour selon la latitude
plot_latitudes: generate_solar_data
	@echo "--- Génération du graphique selon la latitude ---"
	@$(PYTHON_CMD) tools/latitude_plotter.py

plot_duration_heatmap: generate_solar_data
	@echo "--- Génération de la carte thermique annuelle ---"
	@$(PYTHON_CMD) tools/duration_heatmap.py

plot_all: plot_duration plot_solar_models plot_latitudes plot_duration_heatmap

# ==========================================================================
# CIBLES DE MAINTENANCE ET DE QUALITÉ
# ==========================================================================

clean:
	@echo "🧹 Nettoyage : Suppression des exécutables, objets, journaux et données générées..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(COVERAGE_REPORT_DIR)
	@rm -f web/solar_core.mjs web/solar_core.wasm
	@rm -f *.o *.a *.so *.dll *.exe \
		$(LOG_RUNTIME_FILE) \
		$(CSV_OUTPUT_DATA) $(PLOT_DURATION_OUTPUT) $(PLOT_LATITUDES_OUTPUT) \
		$(OUTPUT_DIR)/solar_duration_model_differences.png \
		$(PLOT_DURATION_HEATMAP_OUTPUT) \
		$(SOLAR_MODEL_CSV) $(SOLAR_MODEL_PLOT) $(SOLAR_DATA_LOG)
	@rmdir $(OUTPUT_DIR) 2>/dev/null || true

format:
	@echo "🎨 Formatage du code avec clang-format..."
	@clang-format -i $(APP_SOURCES) $(PUBLIC_HEADERS)

lint:
	@which clang-tidy > /dev/null || { echo "Erreur: clang-tidy non installé. Veuillez l'installer."; exit 1; }
	@echo "🔍 Lancement de l'analyse statique avec clang-tidy..."
	@clang-tidy --quiet $(LINT_SOURCES) -- $(CFLAGS)
	
cppcheck:
	@which cppcheck > /dev/null || { echo "Erreur: cppcheck non installé. Veuillez l'installer..."; exit 1; }
	@echo "🔬 Lancement de l'analyse structurelle avec cppcheck..."
	@cppcheck $(CPPCHECK_ARGS) -Iinclude -Itests/support \
		$(CPPCHECK_SOURCES)

help:
	@echo "============================================================"
	@echo "         🛠️ GUIDE D'UTILISATION DU MAKEFILE 'solar_duration'"
	@echo "============================================================"
	@echo ""
	@echo "🚀 EXÉCUTION & TEST :"
	@echo "  make all / make $(APP_EXECUTABLE) : Compile l'application principale."
	@echo "  make run                   : Exécute la simulation en console (affichage immédiat)."
	@echo "  make run_log               : Exécute la simulation et sauvegarde tout dans $(LOG_RUNTIME_FILE)."
	@echo "  make test                  : Compile et exécute les tests unitaires et d'intégration."
	@echo "  make test_unit             : Exécute uniquement les tests unitaires."
	@echo "  make test_integration      : Exécute le programme avec des configurations isolées."
	@echo "  make coverage              : Mesure les sources C de production et écrit les rapports dans coverage/."
	@echo "  make web_wasm              : Compile le noyau C pour le web (Emscripten requis)."
	@echo "  make run_web               : Compile le noyau et lance le serveur web (port $(WEB_PORT))."
	@echo "  make test_web_wasm         : Teste le module WebAssembly avec Node.js."
	@echo "  make generate_solar_data   : Génère le CSV de comparaison des modèles dans $(OUTPUT_DIR)/."
	@echo ""
	@echo "📊 ANALYSE ET TRACAGE (Workflow complet) :"
	@echo "  make plot_duration         : Simule, exporte un CSV et génère '$(PLOT_DURATION_OUTPUT)'."
	@echo "  make plot_solar_models     : Compare les déclinaisons et leurs effets sur la durée."
	@echo "  make plot_latitudes        : Compare la durée du jour selon la latitude."
	@echo "  make plot_duration_heatmap: Génère la carte thermique année/jour."
	@echo "  make plot_all              : Génère tous les graphiques du projet."
	@echo ""
	@echo "🧹 MAINTENANCE & QUALITÉ :"
	@echo "  make clean                 : Supprime les exécutables, objets, journaux et données générées."
	@echo "  make format                : Reformate le code source avec clang-format."
	@echo "  make lint                  : Analyse statique avec clang-tidy (diagnostics dans le terminal)."
	@echo "  make cppcheck              : Analyse structurelle approfondie avec cppcheck (diagnostics dans le terminal)."
	@echo "============================================================"
