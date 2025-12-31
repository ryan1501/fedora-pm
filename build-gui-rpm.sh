#!/bin/bash
# Build script for fedora-pm-gui RPM

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "fedora-pm-gui-rpm.spec" ]; then
    print_error "fedora-pm-gui-rpm.spec not found. Please run this script from the project root."
    exit 1
fi

# Get version from spec file
VERSION=$(grep "^%define version" fedora-pm-gui-rpm.spec | awk '{print $3}')
RELEASE=$(grep "^%define release" fedora-pm-gui-rpm.spec | awk '{print $3}' | sed 's/%{?dist}//')

print_status "Building fedora-pm-gui RPM version $VERSION-$RELEASE"

# Create rpmbuild directory structure
mkdir -p rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Clean previous builds
print_status "Cleaning previous builds..."
rm -rf rpmbuild/BUILD/*
rm -rf rpmbuild/RPMS/* 2>/dev/null || true
rm -rf rpmbuild/SRPMS/* 2>/dev/null || true

# Create source tarball
print_status "Creating source tarball..."
TAR_NAME="fedora-pm-gui-${VERSION}"

# Create a list of files to include
FILES_TO_INCLUDE="fedora-pm-gui.py fedora-pm-gui-rpm.spec fedora-pm.desktop requirements.txt setup.py fedora_pm_gui/"

# Add optional files if they exist
[ -f "README.md" ] && FILES_TO_INCLUDE="$FILES_TO_INCLUDE README.md"
[ -f "LICENSE" ] && FILES_TO_INCLUDE="$FILES_TO_INCLUDE LICENSE"
[ -f "fedora-pm-gui-launcher.py" ] && FILES_TO_INCLUDE="$FILES_TO_INCLUDE fedora-pm-gui-launcher.py"

tar -czf "rpmbuild/SOURCES/${TAR_NAME}.tar.gz" \
    --exclude-vcs \
    --exclude="rpmbuild" \
    --exclude="target" \
    --exclude="build" \
    --exclude="dist" \
    --exclude="*.pyc" \
    --exclude="__pycache__" \
    --exclude=".gitignore" \
    --transform "s|^|${TAR_NAME}/|" \
    $FILES_TO_INCLUDE

# Copy spec file to SPECS directory
cp fedora-pm-gui-rpm.spec rpmbuild/SPECS/

# Check if required build tools are available
print_status "Checking build dependencies..."
if ! command -v rpmbuild &> /dev/null; then
    print_error "rpmbuild is not installed. Please install rpm-build package:"
    print_error "sudo dnf install rpm-build"
    exit 1
fi

if ! command -v python3 &> /dev/null; then
    print_error "python3 is not installed."
    exit 1
fi

# Build the RPM
print_status "Building RPM..."
cd rpmbuild
rpmbuild -ba --define "_topdir $(pwd)" SPECS/fedora-pm-gui-rpm.spec

# Check if build was successful
if [ $? -eq 0 ]; then
    print_status "RPM build completed successfully!"
    
    # Find the built RPM
    RPM_FILE=$(find RPMS -name "fedora-pm-gui-*.rpm" | head -1)
    SRPM_FILE=$(find SRPMS -name "fedora-pm-gui-*.src.rpm" | head -1)
    
    if [ -n "$RPM_FILE" ]; then
        print_status "Built RPM: $RPM_FILE"
        print_status "You can install it with: sudo dnf install $RPM_FILE"
    fi
    
    if [ -n "$SRPM_FILE" ]; then
        print_status "Built SRPM: $SRPM_FILE"
    fi
else
    print_error "RPM build failed!"
    exit 1
fi

print_status "Build process completed."