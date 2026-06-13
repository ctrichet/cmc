#!/bin/sh
# Test: clipboard mode (-c) with prerequisite guard
set -e

if ! command -v xclip >/dev/null 2>&1 && ! command -v wl-copy >/dev/null 2>&1; then
    echo "SKIP: test_clipboard (neither xclip nor wl-copy available)"
    exit 0
fi

CMC=../cmc
TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "clipboard content" > "$TMPDIR/input.txt"

"$CMC" -c "$TMPDIR/input.txt" 2>/dev/null || {
    echo "FAIL: clipboard mode exited with error"; exit 1;
}

echo "PASS: test_clipboard"
