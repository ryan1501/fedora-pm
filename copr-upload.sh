#!/bin/bash

# Fedora Package Manager - COPR Upload Script (Updated)
# Usage: ./copr-upload.sh <username> [OPTIONS]

set -e

# Configuration
PROJECT_NAME="fedora-pm"
VERSION="1.1.0"
PACKAGER="uncodedchristiangamer"
CHANGELOG="GitHub-based self-update integration with smart API fallbacks and comprehensive RPM distribution"

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

# Check dependencies
check_deps() {
    local missing_deps=()
    
    # Check for fedpkg-packager
    if ! command -v fedpkg-packager >/dev/null 2>&1; then
        missing_deps+=("fedpkg-packager")
    fi
    
    # Check for mock
    if ! command -v mock >/dev/null 2>&1; then
        missing_deps+=("mock")
    fi
    
    # Check for copr-cli
    if ! command -v copr >/dev/null 2>&1; then
        missing_deps+=("copr-cli")
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        error "Missing dependencies: ${missing_deps[*]}"
        log "Install them first: sudo dnf install ${missing_deps[*]}"
        return 1
    fi
    
    success "All dependencies available"
}

# Build packages for multiple Fedora versions
build_packages() {
    local build_count=0
    local success_count=0
    
    log "Building packages for Fedora versions..."
    
    # Create build directory
    local temp_dir="temp-build"
    mkdir -p "$temp_dir"
    local rpms_dir="temp-build/rpms"
    mkdir -p "$rpms_dir"
    
    # Default to current Fedora version
    if [ -z "${BUILD_ALL_VERSIONS}" ]; then
        BUILD_VERSIONS=("fedora-40")
    fi
    
    for fedora_version in "${BUILD_VERSIONS[@]}"; do
        log "Building package for Fedora ${fedora_version}..."
        
        local version_dir="${rpms_dir}/${PROJECT_NAME}-${VERSION}-${fedora_version}"
        local src_dir="${version_dir}/${PROJECT_NAME}-${VERSION}"
        mkdir -p "$src_dir"
        
        # Create spec file
        local spec_file="${src_dir}/${PROJECT_NAME}.copr.spec"
        cat > "$spec_file" << EOF
Name:           ${PROJECT_NAME}
Version:        ${VERSION}
Release:        1%{?dist}
Summary:        Modern package manager for Fedora Linux with GitHub integration
License:        MIT OR Apache-2.0
URL:            https://github.com/ryan1501/fedora-pm
BuildArch:        noarch

BuildRequires:  cargo rust
BuildRequires:       fedpkg-packager
BuildRequires:       mock
Requires:       copr-cli

Requires:       dnf
Requires:       git
Requires:       rpm

%description
A comprehensive package manager for Fedora Linux with GitHub integration.

%prep
%setup -q

%build
cargo build --release

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 target/release/fedora-pm %{buildroot}%{_bindir}/fedora-pm

%files
%{_bindir}/fedora-pm
%doc README.md CHANGELOG_NEW_FEATURES.md LICENSE*

%changelog
* Wed Dec 31 2024 Fedora Package Manager Team <team@fedora-pm.org> - ${VERSION}-1
- COPR upload script with multi-version support
- Enhanced dependency management
- Comprehensive RPM building for all Fedora versions
- Ready for production deployment

EOF
EOF
        
        # Copy source files
        cp -r ./* "$src_dir/" 2>/dev/null || true
        
        # Build with rpmbuild
        if ! rpmbuild -bb \
                --target "${fedora_version}" \
                --define "fedora ${fedora_version}" \
                --define "_ver ${fedora_version}" \
                --define "_release 1%{?dist}" \
                --undefine "_target __target_cpu" \
                --undefine "_target_os linux" \
                --undefine "_licensedir %{_licensedir}" \
                --define "%{datadir %{_datadir}" \
                --define "%_bindir %{_bindir}" \
                --define "%_prefix /usr" \
                --define "_sysconfdir /etc" \
                --define "_localstatedir %{_localstatedir}" \
                --define "_sharedstatedir %{_sharedstatedir}" \
                --define "infodir %{infodir}" \
                --define "_mandir %{_mandir}" \
                --define "_docdir %{docdir}" \
                --define "_htmldir %{_htmldir}" \
                --define "_iconsdir %{_iconsdir}" \
                --define "_desktopdir %{_desktopdir}" \
                --define "_unitdir %{_unitdir}" \
                --define "_appstatedir %{_appstatedir}" \
                --target "${rpms_dir}/${fedora_version}/BUILDROOT" \
                --spec "$spec_file" \
                --nodeps \
                --define "%{licensedir %{_licensedir}" \
                --define "%{datadir %{_datadir}" \
                --define "%{bindir %{_bindir}" \
                --define "%{prefix /usr" \
                --define "_sysconfdir /etc" \
                --define "_localstatedir %{_localstatedir}" \
                --define "_sharedstatedir %{_sharedstatedir}" \
                --define "%{infodir %{infodir}" \
                --define "_mandir %{mandir}" \
                --define "_docdir %{docdir}" \
                --define "_htmldir %{htmldir}" \
                --define "_iconsdir %{iconsdir}" \
                --define "_desktopdir %{desktopdir}" \
                --define "_unitdir %{unitdir}" \
                --define "_appstatedir %{appstatedir}";
        
            if [ $? -eq 0 ]; then
                build_count=$((build_count + 1))
                success "✅ Built Fedora ${fedora_version} package successfully"
            else
                error "❌ Failed to build Fedora ${fedora_version} package"
                failed_count=$((failed_count + 1))
            fi
        done
        
    done
    
    # Collect all built RPMs
    local rpm_files=()
    find "$rpms_dir" -name "*.rpm" | sort
    for rpm_file in "${rpm_files[@]}"; do
        log "Found: $(basename "$rpm_file")"
    done
    
    if [ ${#rpm_files[@]} -eq 0 ]; then
        error "No RPMs were built"
        return 1
    fi
    
    echo "Built ${#rpm_files[@]} RPM files ready for upload"
    return "${rpm_files[@]}"
}

# Upload packages to COPR
upload_to_copr() {
    local rpms_dir="$1"
    local upload_count=0
    local failed_count=0
    
    log "Uploading RPMs to COPR as user: ${PACKAGER}..."
    
    # Check if COPR project exists
    if ! copr list | grep -q "${PROJECT_NAME}" >/dev/null; then
        error "COPR project '${PROJECT_NAME}' does not exist for user ${PACKAGER}"
        log "Create it first at: https://copr.fedorainfracloud.org/coprs/${PACKAGER}/${PROJECT_NAME}/"
        return 1
    fi
    
    # Upload each RPM to COPR
    for rpm_file in "${rpms_dir}" | sort; read -r rpm_file; do
        upload_count=$((upload_count + 1))
        
        log "Uploading $(basename "$rpm_file") to COPR..."
        
        if copr upload \
            --name "${PROJECT_NAME}" \
            --version "${VERSION}" \
            --changelog "${CHANGELOG}" \
            "$rpm_file"; then
            
            upload_count=$((upload_count + 1))
            success "✅ Uploaded $(basename "$rpm_file") to COPR"
        else
            failed_count=$((failed_count + 1))
            error "❌ Failed to upload $(basename "$rpm_file") to COPR"
        fi
    done
    
    log "Upload complete: ${upload_count}/${#rpm_files[@]} RPMs uploaded successfully"
    
    if [ $upload_count -gt $failed_count ]; then
        success "🎉 COPR upload completed successfully!"
        log "Users can install with:"
        echo "sudo dnf copr enable ${PACKAGER}/${PROJECT_NAME}"
        echo "sudo dnf install ${PROJECT_NAME}"
        echo "sudo dnf install ${PROJECT_NAME}-gui"
        return 0
    else
        error "❌ COPR upload failed: ${failed_count}/${#rpm_files[@]} RPMs failed"
        return 1
    fi
    
    # Cleanup
    rm -rf "$rpms_dir"
    
    if [ $upload_count -gt $failed_count ]; then
        success "🎉 COPR upload completed!"
        log "Users can install with:"
        echo "sudo dnf copr enable ${PACKAGER}/${PROJECT_NAME}"
        echo "sudo dnf install ${PROJECT_NAME}"
        echo "sudo dnf install ${PROJECT_NAME}-gui"
    else
        error "❌ Some RPMs failed to upload"
        return 1
    fi
}

# Main execution
main() {
    # Show help if requested
    if [[ "${#:-}" -ge 1 && "${1:-}" == "--help" ]] || [[ "${1:-}" == "-h" ]]; then
        show_help
        exit 0
    fi
    
    # Parse command line arguments
    PARSED_ARGS=()
    USERNAME=""
    VERSION="${VERSION}"
    BUILD_ALL_VERSIONS=""
    DRY_RUN=false
    SOURCE_ONLY=false
    
    while [[ $# -gt 0 ]]; do
        case "$1" in
            --dry-run)
                DRY_RUN=true
                shift
                ;;
            --version)
                VERSION="$2"
                shift
                ;;
            --all)
                BUILD_ALL_VERSIONS="$SUPPORTED_FEDORA_VERSIONS"
                ;;
            --cli-only)
                BUILD_GUI=false
                BUILD_CLI=true
                ;;
            --gui-only
                BUILD_GUI=true
                BUILD_CLI=false
                ;;
            --help|-h)
                show_help
                exit 0
                ;;
            *)
                USERNAME="$1"
                shift
                ;;
        esac
    done
    
    # Check if we have username
    if [ -z "$USERNAME" ]; then
        error "Username required. Usage: $0 <copr-username>"
        show_help
        exit 1
    fi
    
    # Check dependencies
    if ! check_deps; then
        return 1
    fi
    
    # Show dry run info
    if [ "$DRY_RUN" = "true" ]; then
        log "DRY RUN MODE - No actual uploads will occur"
        return 0
    fi
    
    # Build packages
    if [ "$BUILD_ALL_VERSIONS" = "" ] && [ "$BUILD_GUI" = "false" ] && [ "$BUILD_CLI" = "false" ]; then
        BUILD_ALL_VERSIONS="$DEFAULT_VERSION"
    fi
    
    log "Starting COPR upload for ${USERNAME} with versions: ${BUILD_ALL_VERSIONS}"
    
    # Build RPMs
    if ! build_packages; then
        error "Build failed"
        return 1
    fi
    
    # Collect all RPMs
    local rpm_files=($(build_packages))
    
    # Upload to COPR
    upload_to_copr "$rpm_files"
}

# Exit successfully
    exit 0
}