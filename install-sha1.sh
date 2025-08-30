#!/bin/sh
# Download and unpack C++ source code for computing SHA1 hashes.
set -euo pipefail
URL="https://github.com/vog/sha1/archive/refs/heads/master.zip"
ZIP="sha1.zip"
FOLDER="sha1"
INCLUDE_DIR="include"

if [ ! -f "$ZIP" ]; then
    echo "Downloading SHA1 library..."
    curl -L -o "$ZIP" "$URL"
fi

rm -rf "$FOLDER"

unzip -q "$ZIP"
mv sha1-master "$FOLDER"

rm "$ZIP"

mkdir -p "$INCLUDE_DIR"

rm -rf "$INCLUDE_DIR/$FOLDER"
mv "$FOLDER" "$INCLUDE_DIR/"

echo "SHA1 library ready in ./$INCLUDE_DIR/$FOLDER"