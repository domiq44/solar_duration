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

### 1. **run_tests.sh** - Suite de Tests Haute Priorité

Teste les 5 bugs critiques et la qualité du code.

```bash
./run_tests.sh
```

**Couverture** :
- ✅ Validation latitude (Bug #1) - Rejet de 120°
- ✅ Comptage paramètres (Bug #2) - Détection incomplétude
- ✅ Mode invalide (Bug #3) - NULL check protection
- ✅ Configuration valide - Cas nominal
- ✅ Qualité code - Lint & cppcheck

**Résultats attendus** :
```
Test 1: ✓ PASS - Latitude correctly rejected
Test 2: ✓ PASS - Missing parameter correctly detected
Test 3: ✓ PASS - Invalid mode correctly rejected
Test 4: ✓ PASS - Valid configuration runs successfully
Test 5: ✓ PASS - Lint & cppcheck successful
```

### 2. **test_medium_fixes.sh** - Suite de Tests Priorité Moyenne

Teste les quick fixes et refactorings Phase 2.

```bash
./test_medium_fixes.sh
```

**Couverture** :
- ✅ Latitude formatée affichée (Bug #5)
- ✅ Logging cohérent (Bug #4 - log_error)
- ✅ Secondes décimales (Bug #7 - fractional seconds)
- ✅ Lookup tables (Bugs #6, #8)
- ✅ Helper fonction dates (Bug #9)

---

## Comment Exécuter les Tests

### Option 1 : Tests Rapides
```bash
# Test haute priorité uniquement
./run_tests.sh

# Test priorité moyenne uniquement
./test_medium_fixes.sh
```

### Option 2 : Tests + Build Validation
```bash
# Nettoyer, compiler, tester
make clean && make && ./run_tests.sh && ./test_medium_fixes.sh
```

### Option 3 : Tests Individuels Manuels

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
| **Utiliser fichiers test séparés** | Préserve `solar_duration.dat` original avec commentaires/formatage |
| **Sauvegarder/Restaurer** | `cp solar_duration.dat.bak` puis restaurer après test |
| **Capturer output** | `OUTPUT=$(./solar_duration 2>&1)` pour assertions |
| **Tester cas limite** | Lat=90°, Lat=-90°, date inversée, mode invalide |
| **Vérifier compilation** | `make clean && make` avant tests |
| **Documenter intention** | Ajouter commentaires dans scripts de test |

### ❌ À Éviter

| Piège | Impact |
|-------|--------|
| **Modifier `solar_duration.dat` directement** | Perd commentaires, détruit UX |
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
- [ ] `./run_tests.sh` - Tous les tests haute priorité passent
- [ ] `./test_medium_fixes.sh` - Tous les tests priorité moyenne passent
- [ ] `make lint` - 0 violations lint
- [ ] `make cppcheck` - 0 alertes cppcheck
- [ ] `solar_duration.dat` - Format préservé avec commentaires
- [ ] Logs - Niveau approprié (INFO pour production)

---

## Troubleshooting Courant

### Problème : Tests échouent après changement de code

**Solution** :
```bash
make clean && make && ./run_tests.sh
```

### Problème : `Permission denied` sur scripts

**Solution** :
```bash
chmod +x *.sh
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
- [README.md](README.md) - Guide utilisateur
- [Makefile](Makefile) - Cibles de build et test

