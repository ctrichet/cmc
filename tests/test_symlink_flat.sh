#!/bin/sh
# Test: flat mode (without -R) skips symlinks when -s is not specified
# Uses a directory with a regular file and a symlink inside.
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
EXTERNAL=$(mktemp)
trap 'rm -rf "$TMPDIR" "$EXTERNAL"' EXIT

echo "external content" > "$EXTERNAL"
echo "internal file" > "$TMPDIR/actual.txt"
ln -s "$EXTERNAL" "$TMPDIR/link.txt"

# Flat mode without -s: symlink to external file should NOT be followed
output=$("$CMC" "$TMPDIR" 2>/dev/null)
echo "$output" | grep -q "internal file" || { echo "FAIL: regular file should be included"; exit 1; }
echo "$output" | grep -q "external content" && { echo "FAIL: symlink should not be followed without -s"; exit 1; }

# Flat mode with -s: symlink should be followed
output2=$("$CMC" -s "$TMPDIR" 2>/dev/null)
echo "$output2" | grep -q "external content" || { echo "FAIL: symlink should be followed with -s"; exit 1; }

echo "PASS: test_symlink_flat"
