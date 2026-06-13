#!/bin/sh
# Test: error handling for non-existent paths
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

# Non-existent path should produce error on stderr and non-zero exit
if "$CMC" "$TMPDIR/nonexistent.txt" >/dev/null 2>"$TMPDIR/stderr"; then
    echo "FAIL: expected non-zero exit for non-existent path"
    exit 1
fi
if ! grep -q "cmc: error:" "$TMPDIR/stderr"; then
    echo "FAIL: expected 'cmc: error:' on stderr"
    exit 1
fi

# Existing file should produce no stderr
echo "hello" > "$TMPDIR/existing.txt"
: > "$TMPDIR/stderr2"
if ! "$CMC" "$TMPDIR/existing.txt" >/dev/null 2>"$TMPDIR/stderr2"; then
    echo "FAIL: expected zero exit for existing path"
    exit 1
fi
if [ -s "$TMPDIR/stderr2" ]; then
    echo "FAIL: expected empty stderr for existing file"
    exit 1
fi

echo "PASS: test_error"
