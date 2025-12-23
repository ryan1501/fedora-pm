#!/bin/bash
# Build script for Fedora Gaming Meta RPM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="1.0.0"
NAME="fedora-gaming-meta"
BUILD_DIR="rpmbuild"
SOURCES_DIR="$BUILD_DIR/SOURCES"
SPECS_DIR="$BUILD_DIR/SPECS"

echo "Building RPM package for Fedora Gaming Meta..."

# Check for required tools
if ! command -v rpmbuild &> /dev/null; then
    echo "Error: rpmbuild is not installed"
    echo "Please install it with: sudo dnf install rpm-build rpmdevtools"
    exit 1
fi

# Create rpmbuild tree (ensure all directories exist with proper permissions)
mkdir -p "$BUILD_DIR/BUILD" "$BUILD_DIR/BUILDROOT" "$BUILD_DIR/RPMS" "$BUILD_DIR/SOURCES" "$BUILD_DIR/SPECS" "$BUILD_DIR/SRPMS"
# Ensure BUILD directory is writable
chmod 755 "$BUILD_DIR/BUILD" 2>/dev/null || true

# Create source tarball (meta-package doesn't need much, but RPM requires it)
# %setup expects the tarball to extract into a directory named ${NAME}-${VERSION}
echo "Creating source tarball..."
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Create the expected directory structure
mkdir -p "$TEMP_DIR/${NAME}-${VERSION}"

# Copy files into the temp directory
cp fedora-gaming-meta.spec "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp fedora-gaming-meta-README.md "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true

# Create tarball from the temp directory (this ensures proper top-level directory)
cd "$TEMP_DIR"
tar -czf "$SCRIPT_DIR/$SOURCES_DIR/${NAME}-${VERSION}.tar.gz" "${NAME}-${VERSION}"
cd "$SCRIPT_DIR"

# Verify tarball exists
if [ ! -f "$SOURCES_DIR/${NAME}-${VERSION}.tar.gz" ]; then
    echo "Error: Source tarball was not created"
    exit 1
fi
echo "✓ Source tarball created: $SOURCES_DIR/${NAME}-${VERSION}.tar.gz"

# Copy spec file
cp fedora-gaming-meta.spec "$SPECS_DIR/"

# Build RPM
echo "Building RPM..."
rpmbuild --define "_topdir $(pwd)/$BUILD_DIR" \
         -ba "$SPECS_DIR/fedora-gaming-meta.spec"

# Find and display the built RPM
RPM_FILE=$(find "$BUILD_DIR/RPMS" -name "*.rpm" | head -1)
SRPM_FILE=$(find "$BUILD_DIR/SRPMS" -name "*.rpm" | head -1)

if [ -n "$RPM_FILE" ]; then
    echo ""
    echo "✓ RPM built successfully!"
    echo "  Binary RPM: $RPM_FILE"
    if [ -n "$SRPM_FILE" ]; then
        echo "  Source RPM: $SRPM_FILE"
    fi
    echo ""
    echo "To install:"
    echo "  sudo dnf install $RPM_FILE"
    echo ""
    echo "Note: This will install all gaming-related packages."
    echo "      Make sure you have RPM Fusion repositories enabled:"
    echo "      sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-\$(rpm -E %fedora).noarch.rpm"
    echo "      sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-\$(rpm -E %fedora).noarch.rpm"
else
    echo "Error: RPM file not found"
    exit 1
fi

