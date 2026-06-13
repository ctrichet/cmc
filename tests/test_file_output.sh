#!/bin/sh
# Test: file output mode (-o)
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "output content" > "$TMPDIR/in.txt"
OUTFILE="$TMPDIR/out.txt"

"$CMC" -o "$OUTFILE" "$TMPDIR/in.txt" 2>/dev/null

grep -q "output content" "$OUTFILE" || { echo "FAIL: output file missing content"; exit 1; }

echo "PASS: test_file_output"
