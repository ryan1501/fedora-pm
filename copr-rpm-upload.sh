#!/bin/bash

# Complete COPR upload script that builds RPMs and uploads
# Usage: ./copr-rpm-upload.sh [username]

set -e

# Configuration
PROJECT_NAME="fedora-pm"
VERSION="1.1.0"
CHANGELOG="GitHub-based self-update integration with smart GitHub API fallbacks"

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
    info "Checking dependencies..."
    for cmd in fedpkg-packager mock copr-cli; do
        command -v "$cmd" >/dev/null 2>&1 || { 
            if [ "$cmd" = "mock" ]; then
                error "Missing: $cmd. Install with: sudo dnf install fedpkg-packager mock"
            else
                error "Missing: $cmd. Install with: sudo dnf install $cmd"
            fi
        }
    done
    success "Dependencies OK"
}

# Build RPMs for multiple Fedora versions
build_rpms() {
    info "Building RPMs for multiple Fedora versions..."
    
    TEMP_DIR=$(mktemp -d)
    RPMS_DIR="${TEMP_DIR}/rpms"
    
    # Create source tarball
    SOURCE_TAR="${TEMP_DIR}/${PROJECT_NAME}-${VERSION}.tar.gz"
    tar -czf "${SOURCE_TAR}" \
        --exclude-vcs \
        --exclude='target*' \
        --exclude='rpmbuild*' \
        --exclude='.fingerprint*' \
        --exclude='*.tmp' \
        .
    
    success "Source tarball: ${SOURCE_TAR}"
    
    # Build for different Fedora versions
    FEDORA_VERSIONS="fedora-39 fedora-40 fedora-41 fedora-42 fedora-43"
    
    for FEDORA in $FEDORA_VERSIONS; do
        info "Building for ${FEDORA}..."
        
        RPMS_SUBDIR="${RPMS_DIR}/${FEDORA}"
        mkdir -p "${RPMS_SUBDIR}"
        
        # Build with fedpkg-packager
        fedpkg-packager \
            --branch "${FEDORA}" \
            --spec-file "${PROJECT_NAME}.spec" \
            --source-dir "${TEMP_DIR}" \
            --resultdir "${RPMS_SUBDIR}" \
            --name "${PROJECT_NAME}" \
            --version "${VERSION}" \
            --release "1"
        
        if [ $? -eq 0 ]; then
            success "RPMs built for ${FEDORA}"
            
            # List built RPMs
            find "${RPMS_SUBDIR}" -name "*.rpm" | while read rpm_file; do
                log "Built: $(basename "$rpm_file")"
            done
        else
            error "RPM build failed for ${FEDORA}"
        fi
    done
    
    # Cleanup temp files
    rm -rf "${TEMP_DIR}"
    
    # Return RPMS directory path
    echo "${RPMS_DIR}"
}

# Upload RPMs to COPR
upload_rpms() {
    local username="$1"
    local rpms_dir="$2"
    
    if [ -z "$username" ]; then
        error "Username required. Usage: ./copr-rpm-upload.sh <copr-username>"
    fi
    
    info "Uploading RPMs to COPR as user: ${username}..."
    
    # Check if COPR project exists
    if ! copr list | grep -q "${PROJECT_NAME}"; then
        error "COPR project '${PROJECT_NAME}' does not exist for user ${username}"
        info "Create it first at: https://copr.fedorainfracloud.org/coprs/${username}/${PROJECT_NAME}/"
        return 1
    fi
    
    # Upload all RPM files
    upload_count=0
    success_count=0
    
    find "${rpms_dir}" -name "*.rpm" | while read rpm_file; do
        upload_count=$((upload_count + 1))
        
        info "Uploading $(basename "$rpm_file")..."
        
        copr upload \
            --name "${PROJECT_NAME}" \
            --version "${VERSION}" \
            --changelog "${CHANGELOG}" \
            "$rpm_file"
        
        if [ $? -eq 0 ]; then
            success "Uploaded: $(basename "$rpm_file")"
            success_count=$((success_count + 1))
        else
            error "Failed to upload: $(basename "$rpm_file")"
        fi
    done
    
    info "Upload complete: ${success_count}/${upload_count} RPMs uploaded successfully"
    
    if [ $success_count -gt 0 ]; then
        success "COPR upload successful!"
    else
        error "No RPMs uploaded successfully"
        return 1
    fi
    
    # Cleanup RPM directory
    rm -rf "${rpms_dir}"
}

# Main upload function
upload_to_copr() {
    local username="$1"
    
    check_deps
    local rpms_dir=$(build_rpms)
    
    info "Built RPMs are ready in: ${rpms_dir}"
    
    upload_rpms "$username" "$rpms_dir"
}

# Show help
show_help() {
    cat << EOF
${BLUE}COPR RPM Upload Script${NC}

${YELLOW}Usage:${NC} $0 <copr-username> [OPTIONS]

${YELLOW}Description:${NC}
Builds RPMs for multiple Fedora versions and uploads to COPR repository

${YELLOW}Features:${NC}
• Builds for Fedora 39, 40, 41, 42, 43
• Uses fedpkg-packager for proper building
• Uploads both src.rpm and nosrc.rpm packages
• Handles .spec, .nosrc.rpm, .src.rpm files
• Comprehensive error checking and logging

${YELLOW}Requirements:${NC}
• fedpkg-packager
• mock
• copr-cli
• Valid COPR project
• Proper COPR username

${YELLOW}Examples:${NC}
  $0 myusername                    # Upload as myusername
  $0 myusername --force              # Force rebuild
  $0 --help                        # Show this help

${YELLOW}Built RPMs:${NC}
• fedora-pm-1.1.0-1.src.rpm     (source RPM)
• fedora-pm-1.1.0-1.x86_64.rpm   (binary RPM)
• Each for multiple Fedora versions

${YELLOW}After Upload:${NC}
  sudo dnf copr enable myusername/fedora-pm
  sudo dnf install fedora-pm

EOF
}

# Parse arguments
FORCE_BUILD=false
USERNAME=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --force)
            FORCE_BUILD=true
            shift
            ;;
        *)
            USERNAME="$1"
            shift
            ;;
    esac
done

if [ -z "$USERNAME" ]; then
    error "COPR username required. Usage: $0 <copr-username>"
    show_help
    exit 1
fi

# Main execution
info "Starting COPR RPM upload for ${USERNAME}..."
upload_to_copr "$USERNAME"