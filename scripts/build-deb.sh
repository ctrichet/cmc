#!/bin/sh
set -e

VERSION="$1"
ARCH="$2"
OUTDIR="${3:-.}"

if [ -z "$VERSION" ] || [ -z "$ARCH" ]; then
    echo "Usage: $0 <version> <arch> [outdir]"
    exit 1
fi

ROOTDIR=$(mktemp -d)
DEB="cmc_${VERSION}_${ARCH}.deb"
DEBDIR="$ROOTDIR/cmc_${VERSION}_${ARCH}"

mkdir -p "$DEBDIR/DEBIAN"
mkdir -p "$DEBDIR/usr/bin"
mkdir -p "$DEBDIR/usr/share/man/man1"
mkdir -p "$DEBDIR/usr/share/doc/cmc"

cat > "$DEBDIR/DEBIAN/control" <<EOF
Package: cmc
Version: $VERSION
Architecture: $ARCH
Maintainer: cmc contributors
Description: Copy Multiple Contents - collect file contents for LLM context
 A command-line tool that collects the contents of multiple files and outputs
 them to stdout, a file, or the system clipboard. Purpose-built for preparing
 source code and project context for Large Language Models (LLMs).
Depends: libmagic1 (>= 1:5.0)
Recommends: xclip, wl-clipboard
Section: utils
Priority: optional
License: GPL-3.0+
Homepage: https://github.com/ctrichet/cmc
EOF

cp cmc "$DEBDIR/usr/bin/cmc"
cp man/cmc.1 "$DEBDIR/usr/share/man/man1/cmc.1"
cp .cmc_excludes.example "$DEBDIR/usr/share/doc/cmc/cmc_excludes.example"
gzip -9 "$DEBDIR/usr/share/man/man1/cmc.1"

chmod 755 "$DEBDIR/usr/bin/cmc"
chmod 644 "$DEBDIR/DEBIAN/control"
chmod 644 "$DEBDIR/usr/share/man/man1/cmc.1.gz"
chmod 644 "$DEBDIR/usr/share/doc/cmc/cmc_excludes.example"

dpkg-deb --build "$DEBDIR" "$OUTDIR/$DEB" >/dev/null
rm -rf "$ROOTDIR"
echo "$OUTDIR/$DEB"
