# 🏗️ Architecture Technique - Solar Duration Project

**Document de référence technique** pour développeurs. Pour présentation générale, voir `../README.md`. Pour axes d'amélioration, voir `IMPROVEMENTS.md`.

## Organisation du Projet

```text
src/      Implémentations C et point d'entrée main.c
include/  Interfaces publiques et constantes partagées
tests/    Tests unitaires C
docs/     Documentation technique, roadmap et guide de tests
```

Le `Makefile` ajoute `include/` au chemin d'inclusion et construit séparément
le programme `solar_duration` et le runner `solar_duration_tests`.

---

## 📐 Diagramme de Dépendances Modulaires

```
┌─────────────────────────────────────────────────────────────────┐
│                         main.c                                  │
│                    (Orchestrateur)                              │
└──────┬──────────────────────────────┬──────────────────────────┘
       │                              │
       ├─────────────────────────────────────────────┐
       │                              │              │
       │                              │              │
       v                              v              v
   ┌────────────────┐          ┌─────────────┐  ┌────────────┐
   │ config_reader  │          │config_     │  │simulation  │
   │    (RawConfig) │          │validator   │  │  (run loop)│
   └────────┬───────┘          │(FinalConfig)  └──────┬─────┘
            │                  └────────┬──────────────┘
            │                           │
            └───────────────┬───────────┘
                            │
            ┌───────────────┴─────────────────┬──────────────┐
            │                                 │              │
            v                                 v              v
      ┌──────────────┐              ┌──────────────┐  ┌────────────┐
      │date module   │              │solar module  │  │geo module  │
      │(conversions) │              │(declination) │  │(parsing)   │
      └──────┬───────┘              └──────┬───────┘  └────────┬───┘
             │                             │                  │
      (uses)│                             │(uses)        (uses)│
             │                             │                  │
             └──────────────┬──────────────┴──────────────────┘
                            │
                ┌───────────┼───────────┐
                │           │           │
                v           v           v
            ┌─────────┐ ┌────────┐ ┌──────────────┐
            │config.h │ │logger  │ │string_utils  │
            │(consts) │ │(output)│ │(strtrim)     │
            └─────────┘ └────────┘ └──────────────┘
                ▲
        (tous les modules dépendent)
```

---

## 🔄 Flux de Données: Cycle de Vie Complet

```
INPUT FILES
    ▼
┌──────────────────────────────────┐
│ 1. READ PHASE                    │
│  - config_reader.c               │
│  - Lit solar_duration.dat         │
│  - Crée RawConfig (chaînes)      │
│  ✓ Erreurs I/O gérées            │
└──────────────┬───────────────────┘
               │ RawConfig
               ▼
┌──────────────────────────────────┐
│ 2. PARSE PHASE                   │
│  - config_validator.c             │
│  - Convertit chaînes → nombres    │
│  - geo.c: latitude text→double    │
│  ✓ Erreurs format gérées         │
└──────────────┬───────────────────┘
               │ (partiellement validée)
               ▼
┌──────────────────────────────────┐
│ 3. VALIDATE PHASE                │
│  - config_validator.c             │
│  - Vérifie bornes (année,lat)     │
│  - Valide plage de dates          │
│  - Initialise declination_func    │
│  ✓ Erreurs logique gérées        │
└──────────────┬───────────────────┘
               │ FinalConfig (valide)
               ▼
┌──────────────────────────────────┐
│ 4. SIMULATE PHASE                │
│  - simulation.c: run_simulation   │
│  - Boucle jour par jour           │
│  - Appel process_day() pour c/jour│
│  - solar.c: calcul déclinaison    │
│  - Calcul durée du jour           │
│  - Logging résultats              │
└──────────────┬───────────────────┘
               │
               ▼
        OUTPUT (console + log file)
```

---

## 🔌 Interfaces Publiques Clés

### **config_reader.h**

```c
// Allocation structure brute
RawConfig* create_raw_config(void);

// Lecture et extraction clés du fichier .dat
int read_raw_data(const char *filename, RawConfig *raw);
// Retourne: READ_SUCCESS, READ_ERROR_IO_FILE_NOT_FOUND, etc.

// Nettoyage mémoire
void free_raw_config(RawConfig *raw);
```

### **config_validator.h**

```c
// Conversion + validation brute → finale
int parse_and_validate_config(const RawConfig *raw, FinalConfig *final);
// Retourne: READ_SUCCESS ou code erreur

// Vérification logique complète (après parse)
bool is_config_fully_valid(const FinalConfig *final);
// Vérifie: bornes, fonction déclinaison initialisée, dates valides

// Gestion erreurs avec logging
void handle_read_error(int error_code);
```

### **date.h**

```c
// Convertir date → numéro ordinal du jour (1-366)
int date_to_ordinal(int day, int month, int year);

// Vérifier si date existe (ex: 30 février = false)
bool is_date_valid(int day, int month, int year);

// Vérifier si année bissextile
bool is_leap_year(int year);

// Avancer au jour suivant (gère changement mois/année)
void advance_day(int *jour, int *mois, int *annee);
```

### **geo.h**

```c
// Parser "47 deg 17 min 48 sec Nord" → 47.296667
int parse_latitude_string(const char *ligne, double *result);

// Formater latitude double en chaîne lisible
void format_latitude(double lat, char *buf, size_t size);
```

### **solar.h**

```c
// Calcul déclinaison solaire (mode Sinusoïdal)
double calculate_sinusoidal_declination(int jour_n, int annee);

// Calcul déclinaison solaire (mode Spencer - recommandé)
double calculate_spencer_declination(int jour_n, int annee);

// Calcul déclinaison solaire (mode Meeus)
double calculate_meeus_declination(int jour_n, int annee);

// Calcul durée du jour (heures) en fonction latitude et déclinaison
double calculate_day_duration(double delta_deg, double latitude_deg);
```

### **simulation.h**

```c
// Orchestration complète: boucle jour par jour
void run_simulation(const FinalConfig *final);

// process_day() est une fonction interne à simulation.c.
```

### **logger.h**

```c
// Initialisation (lecture solar_duration.cfg)
bool logger_init(const char *config_filename);

// Logs multi-niveaux
void log_debug(const char *format, ...);
void log_info(const char *format, ...);
void log_warn(const char *format, ...);
void log_error(const char *format, ...);
void log_critical(const char *format, ...);
```

---

## 🎨 Design Patterns Appliqués

### **1. Strategy Pattern : Déclinaison Solaire**

**Problème** : Plusieurs algorithmes de calcul, choix à runtime

**Solution** : Pointeur de fonction dans FinalConfig

```c
// config_validator.c
typedef double (*declination_func_t)(int day_of_year);

struct FinalConfig {
  // ...
  declination_func_t declination_func;  // ← Strategy pattern
};

// Sélection dynamique selon mode
switch (mode) {
  case MODE_SINUSOIDAL:
    final->declination_func = calculate_sinusoidal_declination;
    break;
  case MODE_SPENCER:
    final->declination_func = calculate_spencer_declination;
    break;
  // etc.
}
```

**Usage** :
```c
// simulation.c
double declination = final->declination_func(day_of_year);
```

**Avantage** : Ajout nouveau modèle = 1 fonction + 1 case statement

---

### **2. Validator Chain Pattern : Validation Cascade**

**Problème** : Nombreuses étapes de validation (I/O → Format → Logique)

**Solution** : Chaîne d'étapes, chacune générant codes d'erreur

```
read_raw_data()
    ↓ (réussit)
parse_and_validate_config()
    ├─ Conversion geo.c: parse_latitude_string()
    ├─ Conversion date validation
    ├─ Conversion mode_solaire
    ↓ (réussit)
is_config_fully_valid()
    ├─ Vérifier bornes année/latitude
    ├─ Vérifier dates valides et ordonnées
    ├─ Vérifier declination_func != NULL
    ↓ (réussit → Go simulation)
```

**Propriété** : Erreur précoce = diagnostic précis

---

### **3. Factory Pattern : Initialisation Config**

**Localisation** : `config_validator.c::initialize_config_functions()`

```c
// Crée structures et initialise function pointers
bool initialize_config_functions(FinalConfig *final, int mode_solaire) {
  final->declination_func = NULL;  // Default
  
  switch (mode_solaire) {
    case MODE_SINUSOIDAL:
      final->declination_func = calculate_sinusoidal_declination;
      return true;
    // ...
  }
  return false;  // Mode invalide
}
```

---

## 💾 Considérations Mémoire & Performance

### **Allocations Dynamiques**

| Struct | Allouée par | Libérée par | Taille |
|--------|------------|------------|--------|
| `RawConfig` | `create_raw_config()` | `free_raw_config()` | 7 pointers (~56 bytes) + 7 strings |
| `FinalConfig` | Stack (main.c) | Automatic | ~64 bytes |
| Latitude string | `main.c` via `format_latitude()` | `main.c` after logging | ~50 bytes |

**Pattern** : Allocation au démarrage, libération à la fin (pas de leak dans simulation loop)

### **Optimisations Possibles**

| Étape | Complexité Actuelle | Potentiel Amélioration |
|-------|-------------------|----------------------|
| `date_to_ordinal()` | O(12) itération mois | O(1) lookup array |
| Validation date range | O(1) × 3 appels | Cacher résultats |
| Parsing log level | O(5) strcmp | Lookup table |
| Simulation loop | O(N) où N=jours | Pas d'amélioration (nécessaire itérer) |

**Note** : Performance non critique pour domaine (N ≤ 366 jours/run)

### **Stack vs Heap**

- **Heap** : RawConfig uniquement (variable size strings)
- **Stack** : FinalConfig + boucles locales (< 1KB total)
- **Recommandation** : Acceptable, pas de profiling nécessaire

---

## 🧪 Architecture Testing

### **Test Pyramid**

```
                    ▲
                   ╱ ╲
                  ╱   ╲  Integration Tests (workflow)
                 ╱  E2E ╲ - Full pipeline (read→validate→simulate)
                ╱───────╲ - Config files valid/invalid
               ╱ ╱─────╲ ╲ Unit Tests (modules)
              ╱ ╱ Unit  ╲ ╲ - date.c: date_to_ordinal()
             ╱ ╱  Tests  ╲ ╲ - geo.c: parse_latitude_string()
            ╱_╱___________╲_╲ - solar.c: declination models
           ─────────────────── - logger.c: log levels
            Static Analysis
           - clang-tidy (style/logic)
           - cppcheck (memory/structural)
```

### **Unité Test Priorities**

**High Priority** :
- `date_to_ordinal()` : Année bissextile edge cases
- `parse_latitude_string()` : Formats Nord/Sud, boundary 90°
- `calculate_day_duration()` : Polaire, équateur, solstices

**Medium Priority** :
- Logger multi-levels
- Config validator error codes

**Low Priority** :
- String utils (simple)

### **Integration Test Scenarios**

1. **Happy Path** : Valid .dat, complete simulation
2. **Missing Parameter** : .dat sans clé requise → READ_ERROR_DATA_MISSING
3. **Invalid Format** : latitude = "abc" → READ_ERROR_DATA_CONVERSION
4. **Out of Bounds** : Year = 5000 → is_config_fully_valid() = false
5. **Latitude Edge** : ±90.0° (boundary), > 90° (reject)
6. **Invalid Date** : Feb 30 → is_date_valid() = false
7. **Reversed Date Range** : start > end → error

---

## 🔧 Debugging & Profiling

### **Logging Strategy**

Le projet utilise multi-level logging configurable :

```
solar_duration.cfg:
level = DEBUG    ← Very verbose (all function calls)
level = INFO     ← Normal (workflow milestones)
level = WARN     ← Warnings (suspicious but valid)
level = ERROR    ← Errors (failure but continue)
level = CRITICAL ← Fatal (abort)
```

**Usage** :
```bash
# Standard run with INFO level
make run

# Verbose debugging
echo "level = DEBUG" > solar_duration.cfg
make run_log

# Check all output in file
cat solar_duration.log
```

### **Static Analysis**

```bash
make lint       # clang-tidy: style + logic bugs
make cppcheck   # cppcheck: memory safety + structural
make format     # clang-format: consistency
```

**Pré-commit hook suggestion** :
```bash
#!/bin/bash
make lint || exit 1
make cppcheck || exit 1
```

### **GDB Debugging Points**

**Utiles breakpoints** :

```
b main.c:25              # Après logger_init
b config_reader.c:60     # Lors lecture raw data
b config_validator.c:40  # Lors parsing
b simulation.c:70        # Boucle simulation début
```

**Inspect** :
```gdb
(gdb) print *raw         # Voir RawConfig
(gdb) print *final       # Voir FinalConfig
(gdb) print declination  # Valeur déclinaison calculée
```

---

## 📦 Dependencies & Portabilité

### **Dépendances Externes**

| Lib | Utilisée Par | Raison | Obligatoire |
|-----|------------|--------|------------|
| `libm` | solar.c | sin(), cos(), atan(), sqrt() | ✅ Oui |
| `libc` | Tous | stdio, stdlib, string | ✅ Oui |

**Compilation** : `-lm` flag requis (voir Makefile)

### **Portabilité**

- **OS** : Linux/Unix/macOS (utilise POSIX)
- **Compiler** : GCC, Clang (C99 standard)
- **Architecture** : x86_64, ARM (aucune dépendance ISA)

**Note** : Code portable, fonctionnaire sur n'importe quel POSIX

---

## 🚀 Points d'Extension Future

### **Ajouter Nouveau Modèle Déclinaison**

1. Implémente fonction dans `src/solar.c` :
   ```c
  double calculate_custom_declination(int day_of_year, int year) {
     // Formula...
     return declination;
   }
   ```

2. Ajoute mode constant dans `include/config.h` :
   ```c
   #define MODE_CUSTOM (4)
   ```

3. Ajoute case dans `config_validator.c::initialize_config_functions()` :
   ```c
   case MODE_CUSTOM:
    final->declination_func = calculate_custom_declination;
     return true;
   ```

4. Documente dans `README.md` section modèles

### **Ajouter Output Format**

Actuellement : Console + optional log file

Futur : CSV, JSON, Database

**Extension Point** : `simulation.c::process_day()` → Add output handler switch

### **Multi-Locale Support**

Actuellement : Français uniquement

**Extension Points** :
- `src/logger.c` : Localize log messages
- `src/geo.c` : Support "47 degrees..." (English)

---

## 🎯 Architecture Decision Records (ADRs)

### **ADR-001 : Pourquoi Strategy Pattern pour Modèles Solaires?**

**Contexte** : 3 modèles différents, extensible

**Décision** : Pointeur fonction dans FinalConfig

**Rationale** :
- ✅ Zéro overhead à runtime (fonction pointer dispatch)
- ✅ Facile ajouter nouveau modèle
- ✅ Pas besoin runtime polymorphism (C n'a pas de classes)
- ❌ Perte type safety (pas de validation modèle invalid)

**Alternative Rejetée** : `switch()` dans `calculate_day_duration()` → Moins extensible

---

### **ADR-002 : RawConfig vs FinalConfig**

**Contexte** : Besoin 2 étapes (lecture brute → validation)

**Décision** : Structures séparées + conversion explicite

**Rationale** :
- ✅ Clarté flow (Read → Parse → Validate → Simulate)
- ✅ Erreurs détectées au point exact
- ✅ Pas d'état intermédiaire confus
- ❌ Duplication structure légère

---

### **ADR-003 : Logging vs Assertion**

**Contexte** : Erreurs sévères mais non-crash (ex: malloc fail)

**Décision** : `log_critical()` + `return error_code`

**Rationale** :
- ✅ Produit graceful (message clear à utilisateur)
- ✅ Pas d'abort() inopéré (débogage difficile)
- ✅ Testable (error codes trackable)
- ❌ Plus verbose que assert

---

## 📚 Ressources de Référence

### **Astronomical Algorithms**

- Jean Meeus, "Astronomical Algorithms" (2nd ed., 1998)
- Spencer formula source: "Solar Engineering of Thermal Processes"

### **C Best Practices**

- "C Programming: A Modern Approach" - K.N. King
- CERT C Coding Standard (memory safety)
- Clang/GCC diagnostic flags: `-Wall -Wextra -pedantic`

---

**Dernière mise à jour**: 2026-09-05  
**Statut**: Architecture stable, voir IMPROVEMENTS.md pour évolutions
