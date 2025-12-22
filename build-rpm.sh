#!/bin/bash
# Build script for Fedora Package Manager RPM

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="1.0.0"
NAME="fedora-pm"
BUILD_DIR="rpmbuild"
SOURCES_DIR="$BUILD_DIR/SOURCES"
SPECS_DIR="$BUILD_DIR/SPECS"
BUILDROOT_DIR="$BUILD_DIR/BUILDROOT"

echo "Building RPM package for Fedora Package Manager..."

# Check for required tools
if ! command -v rpmbuild &> /dev/null; then
    echo "Error: rpmbuild is not installed"
    echo "Please install it with: sudo dnf install rpm-build"
    exit 1
fi

# Create build directories
mkdir -p "$SOURCES_DIR" "$SPECS_DIR" "$BUILDROOT_DIR"

# Create source tarball
echo "Creating source tarball..."
tar -czf "$SOURCES_DIR/${NAME}-${VERSION}.tar.gz" \
    --exclude='rpmbuild' \
    --exclude='.git' \
    --exclude='*.rpm' \
    --exclude='*.spec.bak' \
    fedora-pm.py \
    fedora-pm-gui.py \
    fedora_pm.py \
    fedora-pm.desktop \
    README.md \
    requirements.txt \
    install.sh

# Copy spec file
cp fedora-pm.spec "$SPECS_DIR/"

# Build RPM
echo "Building RPM..."
rpmbuild --define "_topdir $(pwd)/$BUILD_DIR" \
         --define "_builddir %{_topdir}/BUILD" \
         --define "_rpmdir %{_topdir}/RPMS" \
         --define "_srcrpmdir %{_topdir}/SRPMS" \
         -ba "$SPECS_DIR/fedora-pm.spec"

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
else
    echo "Error: RPM file not found"
    exit 1
fi

