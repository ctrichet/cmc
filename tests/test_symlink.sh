#!/bin/sh
# Test: symlink following (-s)
# Without -s: symlinks are skipped (FTW_PHYS).
# With -s: nftw follows symlinks and includes the target content.
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
EXTERNAL=$(mktemp)
trap 'rm -rf "$TMPDIR" "$EXTERNAL"' EXIT

echo "external content" > "$EXTERNAL"
ln -s "$EXTERNAL" "$TMPDIR/link.txt"
echo "internal file" > "$TMPDIR/actual.txt"

# Without -s: symlink to external file should NOT be followed
output=$("$CMC" -R "$TMPDIR" 2>/dev/null)
echo "$output" | grep -q "internal file" || { echo "FAIL: internal file should be found"; exit 1; }
echo "$output" | grep -q "external content" && { echo "FAIL: symlink target should not be included without -s"; exit 1; }

# With -s: symlink should be followed
output2=$("$CMC" -R -s "$TMPDIR" 2>/dev/null)
echo "$output2" | grep -q "external content" || { echo "FAIL: symlink target should be included with -s"; exit 1; }

echo "PASS: test_symlink"
