#!/bin/sh
# Test: -- argument separator
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "dash file" > "$TMPDIR/-v"
echo "normal" > "$TMPDIR/normal.txt"

output=$("$CMC" -- "$TMPDIR/-v" "$TMPDIR/normal.txt" 2>/dev/null)
echo "$output" | grep -q "normal" || { echo "FAIL: normal file should be included"; exit 1; }

echo "PASS: test_separator"
