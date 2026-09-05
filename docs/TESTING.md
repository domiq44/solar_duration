# 🧪 Guide Complet des Tests - Solar Duration

Ce document explique comment exécuter et écrire des tests pour le projet Solar Duration.

## 📋 Table des matières

- [Tests Existants](#tests-existants)
- [Comment Exécuter les Tests](#comment-exécuter-les-tests)
- [Structure des Fichiers de Configuration](#structure-des-fichiers-de-configuration)
- [Écrire des Tests Personnalisés](#écrire-des-tests-personnalisés)
- [Bonnes Pratiques](#bonnes-pratiques)
- [Interprétation des Résultats](#interprétation-des-résultats)

---

## Tests Existants

### `solar_duration_tests` - Suite de tests unitaires C

Compile et exécute les tests unitaires C sans modifier les fichiers de
configuration du projet.

```bash
make test
```

**Couverture** :
- ✅ Parsing de latitude, secondes fractionnaires et hémisphère Sud
- ✅ Validation des dates et changement d’année
- ✅ Conversion et validation de configuration
- ✅ Sélection du modèle de déclinaison
- ✅ Formatage de latitude

**Résultats attendus** :
```
19 tests, 0 failures
```

---

## Comment Exécuter les Tests

### Commande Recommandée

La cible `make test` est le point d'entrée pour les tests unitaires :

```bash
make test
```

La commande s'arrête dès qu'un test échoue et retourne un code non nul.

### Option 1 : Tests Unitaires
```bash
# Tous les tests unitaires
make test
```

### Option 2 : Tests + Build Validation
```bash
# Nettoyer, compiler, tester
make clean && make test
```

### Option 3 : Test d'Intégration Manuel

Les tests unitaires n'exécutent pas `main.c` ni la simulation complète. Pour
tester le workflow avec une configuration personnalisée, utiliser une copie
temporaire de `solar_duration.dat` :

**Tester avec une configuration personnalisée** :
```bash
# 1. Créer un fichier de config test
cat > test_custom.dat << 'EOF'
annee = 2025
jour_debut = 1
mois_debut = 1
jour_fin = 31
mois_fin = 12
latitude = 48 deg 51 min 24 sec Nord
mode_solaire = 2
EOF

# 2. Copier vers solar_duration.dat (avec sauvegarde)
cp solar_duration.dat solar_duration.dat.bak
cp test_custom.dat solar_duration.dat

# 3. Exécuter
./solar_duration

# 4. Restaurer l'original
cp solar_duration.dat.bak solar_duration.dat
rm test_custom.dat solar_duration.dat.bak
```

---

## Structure des Fichiers de Configuration

### Format Standard

```
annee = 2026
jour_debut = 21
mois_debut = 8
jour_fin = 21
mois_fin = 8
latitude = 47 deg 17 min 48 sec Nord
mode_solaire = 2  // 1=Sinusoïdal, 2=Spencer, 3=Meeus
```

### Format avec Commentaires et Espaces (Production)

```
annee = 2026

jour_debut = 21
mois_debut = 8

jour_fin = 21
mois_fin = 8

latitude = 47 deg 17 min 48 sec Nord

mode_solaire = 2  // 1=Sinusoïdal, 2=Spencer, 3=Meeus
```

### Règles de Parsing

| Élément | Règle |
|---------|-------|
| Commentaires | `//` ou `#` ignorent le reste de la ligne |
| Espaces | Automatiquement trimés avant/après valeurs |
| Lignes vides | Ignorées |
| Casse | Paramètres insensibles à la casse (`ANNEE`, `Annee`, `annee` valides) |

### Valeurs Valides

```
annee       : 1900-3000 (année grégorienne)
jour_debut  : 1-31 (jour du mois)
mois_debut  : 1-12 (mois)
jour_fin    : 1-31 (jour du mois)
mois_fin    : 1-12 (mois)
latitude    : -90°0′0″ à 90°0′0″ (deg min sec format)
mode_solaire: 1 (Sinusoïdal), 2 (Spencer), 3 (Meeus)
```

---

## Écrire des Tests Personnalisés

Les tests unitaires sont écrits dans `../tests/tests.c`. Ajouter une fonction de test,
utiliser `check(condition, "description")`, puis l'appeler depuis `main()`.

Exemple :

```c
static void test_example(void) {
    check(date_to_ordinal(1, 1, 2026) == 1,
                "January 1 has ordinal 1");
}
```

Recompiler et exécuter avec :

```bash
make test
```

### Tests d'Intégration Manuels

Les templates shell ci-dessous sont uniquement destinés à des vérifications
manuelles du programme complet. Ils ne remplacent pas les tests unitaires et
doivent toujours restaurer la configuration originale.

### Template de Test Basique

```bash
#!/bin/bash
# test_my_feature.sh - Tester une nouvelle fonctionnalité

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

echo "Testing My Feature..."

# 1. CRÉER fichier de configuration TEST
cat > test_my_feature.dat << 'EOF'
annee = 2026
jour_debut = 21
mois_debut = 8
jour_fin = 21
mois_fin = 8
latitude = 47 deg 17 min 48 sec Nord
mode_solaire = 2
EOF

# 2. SAUVEGARDER config originale
cp solar_duration.dat solar_duration.dat.bak

# 3. COPIER test vers config active
cp test_my_feature.dat solar_duration.dat

# 4. EXÉCUTER et CAPTURER output
OUTPUT=$(./solar_duration 2>&1)

# 5. VÉRIFIER résultat
if echo "$OUTPUT" | grep -q "SIMULATION TERMINÉE"; then
    echo "✓ PASS: My feature works"
else
    echo "✗ FAIL: My feature broken"
fi

# 6. NETTOYER
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak test_my_feature.dat
```

### Template Avancé (Multiple Tests)

```bash
#!/bin/bash
# test_suite_advanced.sh

set -e  # Arrêter si une commande échoue

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"

echo "=========================================="
echo "Advanced Test Suite"
echo "=========================================="

# Backup original
cp solar_duration.dat solar_duration.dat.bak

run_test() {
    local test_name="$1"
    local config_content="$2"
    local expected_pattern="$3"
    
    echo ""
    echo "📋 $test_name"
    
    cat > test_temp.dat << EOF
$config_content
EOF
    
    cp test_temp.dat solar_duration.dat
    OUTPUT=$(./solar_duration 2>&1 || true)
    
    if echo "$OUTPUT" | grep -q "$expected_pattern"; then
        echo "   ✓ PASS"
    else
        echo "   ✗ FAIL"
        echo "   Expected pattern: $expected_pattern"
    fi
    
    rm test_temp.dat
}

# Test 1
run_test "Test Case 1" \
    "annee = 2026
jour_debut = 21
mois_debut = 8
jour_fin = 21
mois_fin = 8
latitude = 47 deg 17 min 48 sec Nord
mode_solaire = 2" \
    "SIMULATION TERMINÉE"

# Test 2
run_test "Test Case 2" \
    "annee = 2026
jour_debut = 1
mois_debut = 1
jour_fin = 31
mois_fin = 12
latitude = 45 deg 0 min 0 sec Nord
mode_solaire = 1" \
    "SIMULATION TERMINÉE"

# Restore original
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak

echo ""
echo "=========================================="
echo "✓ Test suite completed"
echo "=========================================="
```

---

## Bonnes Pratiques

### ✅ À Faire

| Pratique | Raison |
|----------|--------|
| **Tester les fonctions en mémoire** | Évite de modifier les fichiers de configuration |
| **Sauvegarder/Restaurer** | `cp solar_duration.dat.bak` puis restaurer après test |
| **Capturer output d'intégration** | `OUTPUT=$(./solar_duration 2>&1)` pour assertions |
| **Tester cas limite** | Lat=90°, Lat=-90°, date inversée, mode invalide |
| **Vérifier compilation** | `make clean && make` avant tests |
| **Documenter intention** | Ajouter une description à chaque test unitaire |

### ❌ À Éviter

| Piège | Impact |
|-------|--------|
| **Modifier `solar_duration.dat` sans sauvegarde** | Perd commentaires, détruit UX |
| **Pas de sauvegarde** | Difficile de restaurer après crash test |
| **Grep sans échappement** | Patterns regex peuvent être fragiles |
| **Oublier rm après test** | Accumulation fichiers temporaires |
| **Tests dépendants** | Test 2 échoue si Test 1 n'a pas restauré |
| **Pas d'exit code check** | Manque les erreurs silencieuses |

---

## Interprétation des Résultats

### ✅ Output Réussi

```
[2026-09-05 07:46:40] [INFO]: Latitude cible: 47° 17′ 48″ N
[2026-09-05 07:46:40] [INFO]: SIMULATION TERMINÉE AVEC SUCCÈS.
```

**Signification** : Configuration acceptée, simulation exécutée complètement.

### ⚠️ Erreurs Attendues (Tests de Validation)

#### Latitude Invalide
```
[2026-09-05 07:46:40] [ERROR]: Latitude cible: 120° 0′ 0″ N
[2026-09-05 07:46:40] [ERROR]: ERREUR DE DONNEES: Latitude hors plage valide.
[2026-09-05 07:46:40] [ERROR]: Erreur de validation de la configuration finale.
```

#### Mode Invalide
```
[2026-09-05 07:46:40] [ERROR]: ERREUR DE CONVERSION: 'mode_solaire' invalide.
```

#### Paramètre Manquant
```
[2026-09-05 07:46:40] [ERROR]: Configuration incomplète: 6/7 paramètres trouvés.
```

### 🔍 Debugging

**Voir les logs détaillés** :
```bash
./solar_duration 2>&1 | grep "\[ERROR\]"  # Erreurs uniquement
./solar_duration 2>&1 | grep -E "\[INFO\]|\[ERROR\]"  # INFO + ERREURS
```

**Vérifier niveau de log** :
```bash
cat solar_duration.cfg  # Affiche niveau actuel (DEBUG/INFO/WARN/ERROR)
```

**Activer DEBUG** :
```bash
# 1. Modifier solar_duration.cfg
sed -i 's/level = .*/level = DEBUG/' solar_duration.cfg

# 2. Exécuter
./solar_duration 2>&1 | less

# 3. Restaurer
sed -i 's/level = .*/level = INFO/' solar_duration.cfg
```

---

## Checklist Pre-Release

- [ ] `make clean && make` - Compilation sans warnings
- [ ] `make test` - Tous les tests unitaires passent
- [ ] `make lint` - 0 violations lint
- [ ] `make cppcheck` - 0 alertes cppcheck
- [ ] `solar_duration.dat` - Format préservé avec commentaires
- [ ] Logs - Niveau approprié (INFO pour production)

---

## Troubleshooting Courant

### Problème : Tests échouent après changement de code

**Solution** :
```bash
make clean && make test
```

### Problème : Tests affectent `solar_duration.dat`

**Solution** (Restauration manuelle) :
```bash
# Si vous avez le .bak
cp solar_duration.dat.bak solar_duration.dat

# Sinon, recréer format standard
cat > solar_duration.dat << 'EOF'
annee = 2026

jour_debut = 21
mois_debut = 8

jour_fin = 21
mois_fin = 8

latitude = 47 deg 17 min 48 sec Nord

mode_solaire = 2  // 1=Sinusoïdal, 2=Spencer, 3=Meeus
EOF
```

### Problème : Patterns grep ne matchent pas

**Vérification** :
```bash
./solar_duration 2>&1 | grep -i "simulation"  # Case-insensitive
./solar_duration 2>&1 | head -5  # Voir output réel
```

---

## 📚 Ressources Supplémentaires

- [IMPROVEMENTS.md](IMPROVEMENTS.md) - Détails des bugs et improvements
- [ARCHITECTURE.md](ARCHITECTURE.md) - Structure interne du code
- [README.md](../README.md) - Guide utilisateur
- [Makefile](../Makefile) - Cibles de build et test

