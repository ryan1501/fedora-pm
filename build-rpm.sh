#!/bin/bash
# Build script for Fedora Package Manager RPM (CLI + GUI)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="1.0.0"
NAME="fedora-pm"
BUILD_DIR="rpmbuild"
SOURCES_DIR="$BUILD_DIR/SOURCES"
SPECS_DIR="$BUILD_DIR/SPECS"

echo "=========================================="
echo "Building RPM package for Fedora Package Manager"
echo "=========================================="
echo ""

# Check for required tools
if ! command -v rpmbuild &> /dev/null; then
    echo "Error: rpmbuild is not installed"
    echo "Please install it with: sudo dnf install rpm-build rpmdevtools"
    exit 1
fi

# Create rpmbuild tree
mkdir -p "$BUILD_DIR/BUILD" "$BUILD_DIR/BUILDROOT" "$BUILD_DIR/RPMS" "$BUILD_DIR/SOURCES" "$BUILD_DIR/SPECS" "$BUILD_DIR/SRPMS"
chmod 755 "$BUILD_DIR/BUILD" 2>/dev/null || true

# Create source tarball
echo "Creating source tarball..."
TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Create the expected directory structure
mkdir -p "$TEMP_DIR/${NAME}-${VERSION}"

# Copy all necessary files into the temp directory
echo "Collecting source files..."
cp fedora-pm.py "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp fedora-pm-gui.py "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp fedora-pm.desktop "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp README.md "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp requirements.txt "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true
cp install.sh "$TEMP_DIR/${NAME}-${VERSION}/" 2>/dev/null || true

# Create tarball from the temp directory
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
if [ -f "rpmbuild/SPECS/fedora-pm.spec" ]; then
    cp rpmbuild/SPECS/fedora-pm.spec "$SPECS_DIR/"
    echo "✓ Spec file copied"
else
    echo "Error: Spec file not found at rpmbuild/SPECS/fedora-pm.spec"
    exit 1
fi

# Build RPM
echo ""
echo "Building RPM..."
rpmbuild --define "_topdir $(pwd)/$BUILD_DIR" \
         -ba "$SPECS_DIR/fedora-pm.spec"

# Find and display the built RPM
RPM_FILE=$(find "$BUILD_DIR/RPMS" -name "*.rpm" | head -1)
SRPM_FILE=$(find "$BUILD_DIR/SRPMS" -name "*.rpm" | head -1)

if [ -n "$RPM_FILE" ]; then
    echo ""
    echo "=========================================="
    echo "✓ RPM built successfully!"
    echo "=========================================="
    echo "  Binary RPM: $RPM_FILE"
    if [ -n "$SRPM_FILE" ]; then
        echo "  Source RPM: $SRPM_FILE"
    fi
    echo ""
    echo "To install:"
    echo "  sudo dnf install $RPM_FILE"
    echo ""
    echo "After installation:"
    echo "  - CLI: fedora-pm --help"
    echo "  - GUI: fedora-pm-gui (or find 'Fedora Package Manager' in applications menu)"
    echo ""
else
    echo "Error: RPM file not found"
    exit 1
fi

