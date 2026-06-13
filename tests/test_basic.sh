#!/bin/sh
# Test: basic file selection and stdout output
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "hello world" > "$TMPDIR/a.txt"
echo "foo bar" > "$TMPDIR/b.txt"

output=$("$CMC" "$TMPDIR/a.txt" "$TMPDIR/b.txt" 2>/dev/null)

echo "$output" | grep -q "hello world" || { echo "FAIL: missing content a.txt"; exit 1; }
echo "$output" | grep -q "foo bar" || { echo "FAIL: missing content b.txt"; exit 1; }

echo "PASS: test_basic"
