#!/bin/sh
# Test: directory-prefix exclusion with -e "dir/" and interaction with -R
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$TMPDIR/sub" "$TMPDIR/keep"
echo "excluded" > "$TMPDIR/sub/a.txt"
echo "kept" > "$TMPDIR/keep/b.txt"
echo "rootfile" > "$TMPDIR/root.txt"

# Exclusion by directory prefix: -e "TMPDIR/sub/" should exclude sub/ contents
output=$("$CMC" -R "$TMPDIR" -e "sub/" 2>/dev/null)

echo "$output" | grep -q "excluded" && { echo "FAIL: sub/ content should be excluded"; exit 1; }
echo "$output" | grep -q "kept" || { echo "FAIL: keep/ content should be included"; exit 1; }
echo "$output" | grep -q "rootfile" || { echo "FAIL: root file should be included"; exit 1; }

echo "PASS: test_exclusion_selection"
