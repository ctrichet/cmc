#!/bin/sh
# Test: error handling for non-existent paths
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

# Non-existent path should produce non-zero exit
if "$CMC" "$TMPDIR/nonexistent.txt" >/dev/null 2>&1; then
    echo "FAIL: expected non-zero exit for non-existent path"
    exit 1
fi

echo "PASS: test_error"
