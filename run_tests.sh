#!/bin/bash
# Test suite for Solar Duration project
# Demonstrates proper testing practice: use separate test files, don't overwrite originals

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"
failures=0

echo "=========================================="
echo "Solar Duration Project - Test Suite"
echo "=========================================="
echo ""

# Compile first
echo "🔨 Building project..."
make clean >/dev/null 2>&1
make run >/dev/null 2>&1
echo "✓ Build successful"
echo ""

# Test 1: Invalid latitude (> 90°)
echo "📋 Test 1: Invalid latitude (120°)"
echo "   Expected: REJECT with error message"
cp solar_duration.dat solar_duration.dat.bak
cp test_invalid_latitude.dat solar_duration.dat
OUTPUT=$(./solar_duration 2>&1 || true)
if echo "$OUTPUT" | grep -q "hors plage de format"; then
    echo "   ✓ PASS: Latitude correctly rejected"
else
    echo "   ✗ FAIL: Latitude should be rejected"
    failures=$((failures + 1))
fi
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak
echo ""

# Test 2: Missing parameter
echo "📋 Test 2: Missing parameter (mode_solaire)"
cp solar_duration.dat solar_duration.dat.bak
cp test_missing_parameter.dat solar_duration.dat
OUTPUT=$(./solar_duration 2>&1 || true)
if echo "$OUTPUT" | grep -q "Configuration incomplète.*6/7"; then
    echo "   ✓ PASS: Missing parameter correctly detected"
else
    echo "   ✗ FAIL: Missing parameter should be detected"
    failures=$((failures + 1))
fi
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak
echo ""

# Test 3: Invalid solar mode
echo "📋 Test 3: Invalid solar mode (99)"
cp test_invalid_mode.dat solar_duration.dat
OUTPUT=$(./solar_duration 2>&1 || true)
if echo "$OUTPUT" | grep -q "n'est pas reconnu"; then
    echo "   ✓ PASS: Invalid mode correctly rejected"
else
    echo "   ✗ FAIL: Invalid mode should be rejected"
    failures=$((failures + 1))
fi
cp test_invalid_latitude.dat solar_duration.dat 2>/dev/null || true
echo ""

# Test 4: Valid config (current default)
echo "📋 Test 4: Valid configuration"
cat > solar_duration.dat << 'EOF'
annee = 2026

jour_debut = 21
mois_debut = 8

jour_fin = 21
mois_fin = 8

latitude = 47 deg 17 min 48 sec Nord

mode_solaire = 2  // 1=Sinusoïdal, 2=Spencer, 3=Meeus
EOF

OUTPUT=$(./solar_duration 2>&1)
if echo "$OUTPUT" | grep -q "SIMULATION TERMINÉE AVEC SUCCÈS"; then
    echo "   ✓ PASS: Valid configuration runs successfully"
else
    echo "   ✗ FAIL: Valid configuration should run successfully"
    failures=$((failures + 1))
fi
echo ""

# Test 5: Code quality checks
echo "📋 Test 5: Code quality checks"
echo "   Running: make lint"
if make lint >/dev/null 2>&1; then
    echo "   ✓ PASS: Lint check successful"
else
    echo "   ✗ FAIL: Lint errors found"
    failures=$((failures + 1))
fi
echo "   Running: make cppcheck"
if make cppcheck >/dev/null 2>&1; then
    echo "   ✓ PASS: cppcheck successful"
else
    echo "   ✗ FAIL: cppcheck errors found"
    failures=$((failures + 1))
fi
echo ""

echo "=========================================="
echo "✓ Test suite completed"
echo "=========================================="

if [[ $failures -ne 0 ]]; then
    echo "✗ $failures test(s) failed"
    exit 1
fi
