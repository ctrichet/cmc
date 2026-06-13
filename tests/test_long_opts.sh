#!/bin/sh
# Test: --exclude and --excludes long-form options
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "keep" > "$TMPDIR/keep.txt"
echo "exclude_c" > "$TMPDIR/exclude.c"
echo "exclude_o" > "$TMPDIR/exclude.o"

# Test --exclude
output=$("$CMC" "$TMPDIR" --exclude "*.c" 2>/dev/null)
echo "$output" | grep -q "exclude_c" && { echo "FAIL: --exclude should exclude .c files"; exit 1; }
echo "$output" | grep -q "keep" || { echo "FAIL: --exclude should not affect non-matching files"; exit 1; }

# Test --excludes
output2=$("$CMC" "$TMPDIR" --excludes "*.o" 2>/dev/null)
echo "$output2" | grep -q "exclude_o" && { echo "FAIL: --excludes should exclude .o files"; exit 1; }
echo "$output2" | grep -q "keep" || { echo "FAIL: --excludes should not affect non-matching files"; exit 1; }

# Test --exclude=value form
output3=$("$CMC" "$TMPDIR" --exclude="*.txt" 2>/dev/null)
echo "$output3" | grep -q "keep" && { echo "FAIL: --exclude= should exclude .txt files"; exit 1; }

echo "PASS: test_long_opts"
