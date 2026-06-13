#!/bin/sh
# Test: -e exclusion with absolute scan root
# When scanning an absolute path, relative exclusion patterns like "sub/*"
# should match against the scan root prefix.
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$TMPDIR/sub"
echo "excluded" > "$TMPDIR/sub/a.txt"
echo "kept" > "$TMPDIR/keep.txt"

# Use absolute path as scan root with -e exclusion
output=$("$CMC" -R "$TMPDIR" -e "sub/*" 2>/dev/null)

echo "$output" | grep -q "excluded" && { echo "FAIL: sub/a.txt should be excluded"; exit 1; }
echo "$output" | grep -q "kept" || { echo "FAIL: keep.txt should be included"; exit 1; }

echo "PASS: test_exclude_absolute_root"
