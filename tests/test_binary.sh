#!/bin/sh
# Test: binary detection (-b)
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "text file" > "$TMPDIR/text.txt"
# Copy the first 64 bytes of /bin/ls to create a detectable ELF binary
dd if=/bin/ls bs=64 count=1 of="$TMPDIR/binary.bin" 2>/dev/null

output=$("$CMC" "$TMPDIR" 2>/dev/null)

echo "$output" | grep -q "text file" || { echo "FAIL: text file should be included"; exit 1; }
# Without -b, binary content should not appear in output
printf '%s' "$output" | od -A n -t x1 | grep -q '7f 45 4c 46' && { echo "FAIL: binary file should be excluded without -b"; exit 1; }

output2=$("$CMC" -b "$TMPDIR" 2>/dev/null)
printf '%s' "$output2" | od -A n -t x1 | grep -q '7f 45 4c 46' || { echo "FAIL: binary file should be included with -b"; exit 1; }

echo "PASS: test_binary"
