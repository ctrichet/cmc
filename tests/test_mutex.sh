#!/bin/sh
# Test: -c and -o are mutually exclusive (exit code 2)
set -e

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "test" > "$TMPDIR/file.txt"

set +e
"$CMC" -c -o "$TMPDIR/out.txt" "$TMPDIR/file.txt" >/dev/null 2>&1
exit_code=$?
set -e

if [ "$exit_code" -ne 2 ]; then
    echo "FAIL: expected exit code 2 for -c and -o together, got $exit_code"
    exit 1
fi

echo "PASS: test_mutex"
