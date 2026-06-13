#!/bin/sh
# Test: -E exclusion file
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "included" > "$TMPDIR/keep.txt"
echo "excluded" > "$TMPDIR/skip.txt"
echo "*.txt" > "$TMPDIR/.cmcignore"

output=$("$CMC" "$TMPDIR" -E "$TMPDIR/.cmcignore" 2>/dev/null)

echo "$output" | grep -q "excluded" && { echo "FAIL: skip.txt should be excluded"; exit 1; }

echo "PASS: test_exclude_file"
