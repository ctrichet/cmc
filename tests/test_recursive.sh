#!/bin/sh
# Test: recursive directory scan with -R
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$TMPDIR/sub"
echo "deep" > "$TMPDIR/sub/d.txt"
echo "root" > "$TMPDIR/f.txt"

output=$("$CMC" -R "$TMPDIR" 2>/dev/null)

echo "$output" | grep -q "deep" || { echo "FAIL: missing deep content"; exit 1; }
echo "$output" | grep -q "root" || { echo "FAIL: missing root content"; exit 1; }

echo "PASS: test_recursive"
