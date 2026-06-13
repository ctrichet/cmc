#!/bin/sh
# Test: -e exclusion patterns
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "keep" > "$TMPDIR/keep.txt"
echo "exclude" > "$TMPDIR/exclude.txt"
echo "test" > "$TMPDIR/test.c"

output=$("$CMC" "$TMPDIR" -e "*.txt" 2>/dev/null)

echo "$output" | grep -q "keep" && { echo "FAIL: keep.txt should be excluded"; exit 1; }
echo "$output" | grep -q "exclude" && { echo "FAIL: exclude.txt should be excluded"; exit 1; }
echo "$output" | grep -q "test" || { echo "FAIL: test.c should be included"; exit 1; }

echo "PASS: test_exclude"
