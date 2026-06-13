#!/bin/sh
set -e

REPO_DIR="$1"
DIST="${2:-stable}"
SECTION="${3:-main}"

if [ -z "$REPO_DIR" ]; then
    echo "Usage: $0 <repo-dir> [dist] [section]"
    exit 1
fi

mkdir -p "$REPO_DIR/dists/$DIST/$SECTION/binary-amd64"
mkdir -p "$REPO_DIR/dists/$DIST/$SECTION/binary-arm64"

# Copy .deb files to pool
mkdir -p "$REPO_DIR/pool/$SECTION"
find . -name "*.deb" -not -path "./$REPO_DIR/*" -exec cp {} "$REPO_DIR/pool/$SECTION/" \;

# Generate Packages files
for arch in amd64 arm64; do
    (cd "$REPO_DIR" && dpkg-scanpackages --arch "$arch" "pool/$SECTION" > "dists/$DIST/$SECTION/binary-$arch/Packages" 2>/dev/null)
    gzip -9 -c "$REPO_DIR/dists/$DIST/$SECTION/binary-$arch/Packages" > "$REPO_DIR/dists/$DIST/$SECTION/binary-$arch/Packages.gz"
done

# Copy public key before cd
if [ -f "cmc.gpg.asc" ]; then
    cp cmc.gpg.asc "$REPO_DIR/"
fi

# Generate Release file
cd "$REPO_DIR"
cat > "dists/$DIST/Release" <<EOF
Origin: cmc
Label: cmc APT Repository
Suite: $DIST
Codename: $DIST
Date: $(date -Ru)
Architectures: amd64 arm64
Components: $SECTION
Description: cmc - Copy Multiple Contents
EOF

# Add hashes
for hash_type in MD5sum SHA1 SHA256; do
    echo "$hash_type:" >> "dists/$DIST/Release"
    find "dists/$DIST" -type f -not -name "Release" -not -name "Release.gpg" -not -name "InRelease" | sort | while read f; do
        case "$hash_type" in
            MD5sum)  h=$(md5sum "$f" | cut -d' ' -f1);;
            SHA1)    h=$(sha1sum "$f" | cut -d' ' -f1);;
            SHA256)  h=$(sha256sum "$f" | cut -d' ' -f1);;
        esac
        s=$(stat -c%s "$f" 2>/dev/null || stat -f%z "$f" 2>/dev/null)
        echo " $h $s $f" >> "dists/$DIST/Release"
    done
done

# Sign with GPG
if [ -n "$GPG_PRIVATE_KEY" ]; then
    echo "$GPG_PRIVATE_KEY" | gpg --import --batch --passphrase "$GPG_PASSPHRASE" 2>/dev/null || true
    gpg --batch --yes --pinentry-mode loopback --passphrase "$GPG_PASSPHRASE" \
        --detach-sign --armor -o "dists/$DIST/Release.gpg" "dists/$DIST/Release"
    gpg --batch --yes --pinentry-mode loopback --passphrase "$GPG_PASSPHRASE" \
        --clearsign -o "dists/$DIST/InRelease" "dists/$DIST/Release"
fi

