#!/bin/sh
# Test: -v and --version output
set -e

CMC=../cmc

version1=$("$CMC" -v 2>/dev/null)
version2=$("$CMC" --version 2>/dev/null)

echo "$version1" | grep -q "^cmc version" || { echo "FAIL: -v should output version"; exit 1; }
echo "$version2" | grep -q "^cmc version" || { echo "FAIL: --version should output version"; exit 1; }
[ "$version1" = "$version2" ] || { echo "FAIL: -v and --version output should match"; exit 1; }

echo "PASS: test_version"
