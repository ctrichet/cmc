#!/bin/sh
# Test: binary detection (-b)
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "text file" > "$TMPDIR/text.txt"
# Copy the first 64 bytes of /bin/ls to create a detectable ELF binary
dd if=/bin/ls bs=64 count=1 of="$TMPDIR/binary.bin" 2>/dev/null

OUTFILE1=$(mktemp)
trap 'rm -rf "$TMPDIR" "$OUTFILE1"' EXIT

"$CMC" "$TMPDIR" > "$OUTFILE1" 2>/dev/null
grep -q "text file" "$OUTFILE1" || { echo "FAIL: text file should be included"; exit 1; }
# Without -b, binary content should not appear in output
od -A n -t x1 "$OUTFILE1" | grep -q '7f 45 4c 46' && { echo "FAIL: binary file should be excluded without -b"; exit 1; }

OUTFILE2=$(mktemp)
trap 'rm -rf "$TMPDIR" "$OUTFILE1" "$OUTFILE2"' EXIT

"$CMC" -b "$TMPDIR" > "$OUTFILE2" 2>/dev/null
od -A n -t x1 "$OUTFILE2" | grep -q '7f 45 4c 46' || { echo "FAIL: binary file should be included with -b"; exit 1; }

echo "PASS: test_binary"
