#!/bin/sh
# Test: trailing slash in directory argument produces same output as without
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

mkdir -p "$TMPDIR/sub"
echo "deep" > "$TMPDIR/sub/d.txt"
echo "root" > "$TMPDIR/f.txt"

output_no_slash=$("$CMC" -R "$TMPDIR" 2>/dev/null)
output_with_slash=$("$CMC" -R "$TMPDIR/" 2>/dev/null)

if [ "$output_no_slash" != "$output_with_slash" ]; then
    echo "FAIL: output differs with trailing slash"
    echo "--- without slash ---"
    echo "$output_no_slash"
    echo "--- with slash ---"
    echo "$output_with_slash"
    exit 1
fi

echo "$output_no_slash" | grep -q "deep" || { echo "FAIL: missing deep content"; exit 1; }
echo "$output_no_slash" | grep -q "root" || { echo "FAIL: missing root content"; exit 1; }

echo "PASS: test_trailing_slash"
