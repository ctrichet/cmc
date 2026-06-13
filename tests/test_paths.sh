#!/bin/sh
# Test: -p path prefix mode
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "content" > "$TMPDIR/myfile.txt"

output=$("$CMC" -p "$TMPDIR/myfile.txt" 2>/dev/null)

echo "$output" | grep -q "### FILE:" || { echo "FAIL: missing path prefix"; exit 1; }
echo "$output" | grep -q "myfile.txt" || { echo "FAIL: missing filename in prefix"; exit 1; }
echo "$output" | grep -q "content" || { echo "FAIL: missing file content"; exit 1; }

echo "PASS: test_paths"
