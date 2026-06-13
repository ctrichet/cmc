#!/bin/sh
# Test: -E exclusion patterns and .cmc_excludes auto-load
set -e

CMC=../cmc

cleanup() { rm -rf "$TMPDIR" "$TMPDIR2" "$TMPDIR3"; }
trap cleanup EXIT

# --- Test 4.1: -E with patterns + auto-load from XDG_CONFIG_HOME ---
TMPDIR=$(mktemp -d)
mkdir -p "$TMPDIR/xdg-home/cmc"

echo "*.txt" > "$TMPDIR/xdg-home/cmc/.cmc_excludes"
echo "included" > "$TMPDIR/keep.c"
echo "excluded" > "$TMPDIR/skip.txt"

output=$(XDG_CONFIG_HOME="$TMPDIR/xdg-home" "$CMC" "$TMPDIR" -E "*.c" 2>/dev/null)
echo "$output" | grep -q "included" && { echo "FAIL: keep.c should be excluded by -E *.c"; exit 1; }
echo "$output" | grep -q "excluded" && { echo "FAIL: skip.txt should be excluded by .cmc_excludes"; exit 1; }

echo "PASS: test_exclude_file (auto-load + patterns)"

# --- Test 4.2: warning when .cmc_excludes is missing ---
TMPDIR2=$(mktemp -d)

echo "dummy" > "$TMPDIR2/dummy.c"
stderr=$(XDG_CONFIG_HOME="$TMPDIR2" "$CMC" "$TMPDIR2" -E "*.c" 2>&1 >/dev/null)
echo "$stderr" | grep -q "warning: no .cmc_excludes" || { echo "FAIL: missing .cmc_excludes warning not printed"; echo "stderr was: $stderr"; exit 1; }

# Without -E: no warning (even if XDG_CONFIG_HOME points to non-existent cmc dir)
stderr2=$(XDG_CONFIG_HOME="$TMPDIR2" "$CMC" "$TMPDIR2" -e "*.c" 2>&1 >/dev/null)
echo "$stderr2" | grep -q "warning: no .cmc_excludes" && { echo "FAIL: warning should not appear without -E"; exit 1; }

echo "PASS: test_exclude_file_missing_warning"

# --- Test 4.3: -E + -e combined ---
TMPDIR3=$(mktemp -d)

echo "alpha" > "$TMPDIR3/alpha.c"
echo "beta" > "$TMPDIR3/beta.py"
echo "gamma" > "$TMPDIR3/gamma.rs"

output3=$(XDG_CONFIG_HOME="$TMPDIR3" "$CMC" "$TMPDIR3" -e "*.c" -E "*.py" 2>/dev/null)
echo "$output3" | grep -q "alpha" && { echo "FAIL: alpha.c should be excluded by -e"; exit 1; }
echo "$output3" | grep -q "beta" && { echo "FAIL: beta.py should be excluded by -E"; exit 1; }
echo "$output3" | grep -q "gamma" || { echo "FAIL: gamma.rs should be included"; exit 1; }

echo "PASS: test_exclude_E_and_e_combined"
