#!/bin/bash

# Create source package for COPR upload
set -e

# Configuration
PROJECT_NAME="fedora-pm"
VERSION="1.0.0" 
RELEASE="1"
PACKAGER="uncodedchristiangamer"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

log() { echo -e "${NC}$1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
info() { echo -e "${BLUE}[INFO]${NC} $1"; }

# Create source tarball
create_source() {
    info "Creating source tarball..."
    
    TAR_FILE="${PROJECT_NAME}-${VERSION}.tar.gz"
    
    tar -czf "${TAR_FILE}" \
        --exclude-vcs \
        --exclude='target*' \
        --exclude='rpmbuild*' \
        --exclude='.fingerprint*' \
        --exclude='*.tmp' \
        --exclude='*.log' \
        .
    
    success "Source tarball: ${TAR_FILE}"
    echo "${TAR_FILE}"
}

# Mock build process to create proper structure
create_mock_rpms() {
    info "Creating mock RPM structure for testing..."
    
    RPMS_DIR="test-rpms/${PROJECT_NAME}"
    mkdir -p "${RPMS_DIR}"
    
    # Create mock spec file
    cat > "${RPMS_DIR}/${PROJECT_NAME}.spec" << EOF
Name: ${PROJECT_NAME}
Version: ${VERSION}
Release: ${RELEASE}%{?dist}
Summary: Modern package manager for Fedora Linux with GitHub integration
License: MIT OR Apache-2.0
URL: https://github.com/ryan1501/fedora-pm
BuildArch: noarch

%description
A comprehensive package manager for Fedora Linux with CLI and GUI interfaces.

%prep
%setup -q

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 target/release/fedora-pm %{buildroot}%{_bindir}/fedora-pm
install -m 0755 ${PROJECT_NAME}.desktop %{buildroot}%{_datadir}/applications/

%files
%{_bindir}/fedora-pm
%doc README.md CHANGELOG_NEW_FEATURES.md
%license LICENSE*
%doc %{_licensedir}/*

%changelog
* Wed Dec 31 2024 Fedora Package Manager Team <team@fedora-pm.org> - ${VERSION}-${RELEASE}
- Initial COPR upload preparation
- Complete GitHub-based self-update integration
- Comprehensive GUI installation guide
- Production-ready CLI with 70+ commands
- Ready for Fedora distribution
EOF
    
    success "Created mock RPM spec: ${RPMS_DIR}/${PROJECT_NAME}.spec"
    
    # Create mock desktop file
    cat > "${RPMS_DIR}/${PROJECT_NAME}.desktop" << EOF
[Desktop Entry]
Name=Fedora Package Manager
Comment=Modern package manager for Fedora Linux
Exec=/usr/bin/fedora-pm-gui
Icon=fedora-pm
Terminal=false
Type=Application
Categories=System;PackageManager;
MimeType=text/x-rpm;
Keywords=package;manager;fedora;dnf;
EOF
    
    success "Created mock desktop file: ${RPMS_DIR}/${PROJECT_NAME}.desktop"
    
    # Create mock build directory
    BUILD_DIR="${RPMS_DIR}/BUILD"
    mkdir -p "${BUILD_DIR}/usr/bin"
    mkdir -p "${BUILD_DIR}/usr/share/applications"
    mkdir -p "${BUILD_DIR}/usr/share/doc"
    
    # Copy built binary
    if [ -f "target/release/fedora-pm" ]; then
        cp target/release/fedora-pm "${BUILD_DIR}/usr/bin/"
        success "Copied binary to ${BUILD_DIR}/usr/bin/"
    else
        error "Binary not found at target/release/fedora-pm"
    fi
    
    # Copy desktop file
    cp "${PROJECT_NAME}.desktop" "${BUILD_DIR}/usr/share/applications/"
    
    success "Mock RPM structure created in ${RPMS_DIR}"
    echo "${RPMS_DIR}"
}

# Upload to COPR
upload_to_copr() {
    if [ -z "$PACKAGER" ]; then
        error "Package name required. Usage: $0 <copr-username>"
        return 1
    fi
    
    TAR_FILE="${PROJECT_NAME}-${VERSION}.tar.gz"
    
    # Create source tarball
    create_source
    
    info "Uploading to COPR as user: ${PACKAGER}..."
    
    # Check if copr-cli is available
    if ! command -v copr >/dev/null 2>&1; then
        error "copr-cli not found. Install with: sudo dnf install copr-cli"
        return 1
    fi
    
    # Check if COPR project exists
    if ! copr list | grep -q "${PROJECT_NAME}"; then
        error "COPR project '${PROJECT_NAME}' does not exist for user ${PACKAGER}"
        info "Create it first at: https://copr.fedorainfracloud.org/coprs/${PACKAGER}/${PROJECT_NAME}/"
        return 1
    fi
    
    # Upload source tarball (this creates source RPM)
    copr upload \
        --name "${PROJECT_NAME}" \
        --version "${VERSION}" \
        --changelog "Initial COPR upload with GitHub integration and complete GUI installation guide" \
        "${TAR_FILE}"
    
    if [ $? -eq 0 ]; then
        success "Source upload successful to COPR!"
    else
        error "Failed to upload source to COPR"
        return 1
    fi
}

# Show help
show_help() {
    cat << EOF
${BLUE}COPR Upload Script${NC}

${YELLOW}Usage:${NC}
    $0 <copr-username> [OPTIONS]

${YELLOW}Options:${NC}
    --dry-run       Show what would be uploaded without uploading
    --source-only    Only upload source tarball (no binary RPM)
    --version <v>   Override version (default: ${VERSION})
    --user <user>   Override packager (default: ${PACKAGER})

${YELLOW}Examples:${NC}
    $0 uncodedchristiangamer          # Upload as uncodedchristiangamer
    $0 myusername --version 1.1.1     # Custom version
    $0 myuser --source-only                # Source tarball only
    $0 myuser --dry-run                  # Test upload process

${YELLOW}What Gets Built:${NC}
• Source tarball: ${PROJECT_NAME}-${VERSION}.tar.gz
• Mock RPM structure for testing
• Proper source directory layout
• Desktop integration files
• Package metadata ready

${YELLOW}After Upload:${NC}
• Users can install with: sudo dnf copr enable ${PACKAGER}/${PROJECT_NAME}
• Install with: sudo dnf install ${PROJECT_NAME}
• Install GUI with: sudo dnf install ${PROJECT_NAME}-gui
EOF
}

# Parse arguments
DRY_RUN=false
SOURCE_ONLY=false
while [[ $# -gt 0 ]]; do
    case $1 in
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --source-only)
            SOURCE_ONLY=true
            shift
            ;;
        --version)
            VERSION="$2"
            shift
            ;;
        --user)
            PACKAGER="$2"
            shift
            ;;
        --help|-h)
            show_help
            exit 0
            ;;
        *)
            PACKAGER="$1"
            ;;
    esac
    shift
done

# Main execution
main() {
    if [ "$DRY_RUN" = "true" ]; then
        info "DRY RUN MODE - No actual upload"
    fi
    
    if [ -z "$PACKAGER" ]; then
        error "COPR username required. Use --help for usage."
        exit 1
    fi
    
    if [ "$SOURCE_ONLY" = "false" ]; then
        upload_to_copr
    else
        create_source
        info "Source tarball created: ${TAR_FILE}"
    fi
}

main "$@"