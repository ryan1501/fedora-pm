#!/bin/bash
# Install all dependencies from fedora-gaming-meta.spec

# Extract all Requires: packages from the spec file
PACKAGES=$(grep -E "^Requires:" fedora-gaming-meta.spec | sed 's/Requires: //' | tr '\n' ' ')

echo "Installing dependencies from fedora-gaming-meta.spec..."
echo "Packages to install:"
echo "$PACKAGES"
echo ""

sudo dnf install -y $PACKAGES

echo ""
echo "✓ Dependencies installed!"

