#!/bin/bash
# Test fractional seconds in latitude parsing
# Verify Bug #7 fix: Support for decimal seconds like "47 deg 17 min 48.5 sec Nord"

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_DIR"
failures=0

echo "=========================================="
echo "Medium Priority Fixes - Test Suite"
echo "=========================================="
echo ""

# Test 1: Latitude output in logs (Bug #5 fix)
echo "📋 Test 1: Latitude formatted string in logs (Bug #5)"
echo "   Expected: 'Latitude cible: 47° 17′ 48″ N' in output"
OUTPUT=$(./solar_duration 2>&1)
if echo "$OUTPUT" | grep -q "Latitude cible:"; then
    echo "   ✓ PASS: Latitude cible is displayed"
    echo "$OUTPUT" | grep "Latitude cible:"
else
    echo "   ✗ FAIL: Latitude cible should be displayed"
    failures=$((failures + 1))
fi
echo ""

# Test 2: log_error instead of fprintf (Bug #4 fix)
echo "📋 Test 2: Consistent logging (Bug #4 - no direct stderr)"
echo "   Expected: Errors go through log system"
cat > test_bad_mode.dat << 'EOF'
annee = 2026
jour_debut = 21
mois_debut = 8
jour_fin = 21
mois_fin = 8
latitude = 47 deg 17 min 48 sec Nord
mode_solaire = abc
EOF

cp solar_duration.dat solar_duration.dat.bak
cp test_bad_mode.dat solar_duration.dat
OUTPUT=$(./solar_duration 2>&1 || true)
if echo "$OUTPUT" | grep -q "\[ERROR\]"; then
    echo "   ✓ PASS: Error is logged via log system"
else
    echo "   ✗ FAIL: Error should be logged"
    failures=$((failures + 1))
fi
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak test_bad_mode.dat
echo ""

# Test 3: Fractional seconds support (Bug #7 fix)
echo "📋 Test 3: Fractional seconds in latitude (Bug #7)"
echo "   Expected: '47 deg 17 min 48.5 sec Nord' accepted"
cat > test_fractional_seconds.dat << 'EOF'
annee = 2026
jour_debut = 21
mois_debut = 8
jour_fin = 21
mois_fin = 8
latitude = 47 deg 17 min 48.5 sec Nord
mode_solaire = 2
EOF

cp solar_duration.dat solar_duration.dat.bak
cp test_fractional_seconds.dat solar_duration.dat
OUTPUT=$(./solar_duration 2>&1)
if echo "$OUTPUT" | grep -q "SIMULATION TERMINÉE"; then
    echo "   ✓ PASS: Fractional seconds accepted"
    echo "   Parsed latitude: $(echo "$OUTPUT" | grep "Latitude parsée" | head -1)"
else
    echo "   ✗ FAIL: Fractional seconds should be accepted"
    failures=$((failures + 1))
fi
cp solar_duration.dat.bak solar_duration.dat
rm solar_duration.dat.bak test_fractional_seconds.dat
echo ""

# Restore original config
cat > solar_duration.dat << 'EOF'
annee = 2026

jour_debut = 21
mois_debut = 8

jour_fin = 21
mois_fin = 8

latitude = 47 deg 17 min 48 sec Nord

mode_solaire = 2  // 1=Sinusoïdal, 2=Spencer, 3=Meeus
EOF

echo "=========================================="
echo "✓ Medium priority tests completed"
echo "=========================================="

if [[ $failures -ne 0 ]]; then
    echo "✗ $failures test(s) failed"
    exit 1
fi
