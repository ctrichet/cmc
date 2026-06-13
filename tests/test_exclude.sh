#!/bin/sh
# Test: -e exclusion patterns
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR" "$TMPDIR2"' EXIT

echo "keep" > "$TMPDIR/keep.txt"
echo "exclude" > "$TMPDIR/exclude.txt"
echo "test" > "$TMPDIR/test.c"
echo "hello" > "$TMPDIR/main.c"

output=$("$CMC" "$TMPDIR" -e "*.txt" 2>/dev/null)

echo "$output" | grep -q "keep" && { echo "FAIL: keep.txt should be excluded"; exit 1; }
echo "$output" | grep -q "exclude" && { echo "FAIL: exclude.txt should be excluded"; exit 1; }
echo "$output" | grep -q "test" || { echo "FAIL: test.c should be included"; exit 1; }

echo "PASS: test_exclude"

# Test: multiple exclusion patterns with a single -e
TMPDIR2=$(mktemp -d)

echo "alpha" > "$TMPDIR2/alpha.c"
echo "beta" > "$TMPDIR2/beta.py"
echo "gamma" > "$TMPDIR2/gamma.rs"
echo "delta" > "$TMPDIR2/delta.txt"

output2=$("$CMC" "$TMPDIR2" -e "*.c" "*.py" 2>/dev/null)

echo "$output2" | grep -q "alpha" && { echo "FAIL: alpha.c should be excluded"; exit 1; }
echo "$output2" | grep -q "beta" && { echo "FAIL: beta.py should be excluded"; exit 1; }
echo "$output2" | grep -q "gamma" || { echo "FAIL: gamma.rs should be included"; exit 1; }
echo "$output2" | grep -q "delta" || { echo "FAIL: delta.txt should be included"; exit 1; }

echo "PASS: test_exclude_multi"
