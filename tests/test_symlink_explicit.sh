#!/bin/sh
# Test: explicit symlink as selection pattern
# Without -s: symlink argument should be skipped.
# With -s: symlink argument should be followed.
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
EXTERNAL=$(mktemp)
trap 'rm -rf "$TMPDIR" "$EXTERNAL"' EXIT

echo "external content" > "$EXTERNAL"
ln -s "$EXTERNAL" "$TMPDIR/link.txt"
echo "regular file" > "$TMPDIR/actual.txt"

# Without -s: symlink should be skipped
output=$("$CMC" "$TMPDIR/link.txt" 2>/dev/null || true)
echo "$output" | grep -q "external content" && { echo "FAIL: symlink should not be followed without -s"; exit 1; }

# With -s: symlink should be followed
output2=$("$CMC" -s "$TMPDIR/link.txt" 2>/dev/null)
echo "$output2" | grep -q "external content" || { echo "FAIL: symlink should be followed with -s"; exit 1; }

# Regular file should always be included
output3=$("$CMC" "$TMPDIR/actual.txt" 2>/dev/null)
echo "$output3" | grep -q "regular file" || { echo "FAIL: regular file should always be included"; exit 1; }

echo "PASS: test_symlink_explicit"
