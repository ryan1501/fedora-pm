#!/bin/bash

# Quick upload script for fedora-pm to COPR
# Usage: ./upload-to-copr.sh

set -e

# Configuration
PROJECT_NAME="fedora-pm"
VERSION="1.1.0"
CHANGELOG="GitHub-based self-update integration with smart GitHub API integration and fallback mechanisms"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'

log() { echo -e "${NC}$1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }

# Check dependencies
check_deps() {
    info "Checking dependencies..."
    command -v copr-cli >/dev/null 2>&1 || { error "copr-cli not found. Install with: sudo dnf install copr-cli"; }
    success "Dependencies OK"
}

# Create source tarball
prepare_source() {
    info "Creating source tarball..."
    TAR_FILE="${PROJECT_NAME}-${VERSION}.tar.gz"
    
    tar -czf "${TAR_FILE}" \
        --exclude-vcs \
        --exclude='target*' \
        --exclude='rpmbuild*' \
        --exclude='.fingerprint*' \
        --exclude='*.tmp' \
        .
    
    success "Source tarball: ${TAR_FILE}"
    echo "${TAR_FILE}"
}

# Upload to COPR
upload_to_copr() {
    local username="$1"
    
    if [ -z "$username" ]; then
        error "Username required. Usage: ./upload-to-copr.sh <copr-username>"
    fi
    
    prepare_source
    
    info "Uploading to COPR as user: ${username}..."
    
    if ! copr list | grep -q "${PROJECT_NAME}"; then
        warning "COPR project '${PROJECT_NAME}' does not exist for user ${username}"
        info "Please create it first at: https://copr.fedorainfra.org/coprs/${username}/${PROJECT_NAME}/"
    else
        copr upload \
            --name "${PROJECT_NAME}" \
            --version "${VERSION}" \
            --changelog "${CHANGELOG}" \
            "${PROJECT_NAME}-${VERSION}.tar.gz"
        
        if [ $? -eq 0 ]; then
            success "Upload successful to COPR!"
        else
            error "Upload failed"
        fi
    fi
    
    # Cleanup
    rm -f "${PROJECT_NAME}-${VERSION}.tar.gz"
}

info() { echo -e "${BLUE}[INFO]${NC} $1"; }
warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }

# Show help
show_help() {
    cat << EOF
${BLUE}fedora-pm COPR Upload Script${NC}

${YELLOW}Usage:${NC} $0 <copr-username>

${YELLOW}Description:${NC}
Uploads fedora-pm ${VERSION} to COPR repository

${YELLOW}Requirements:${NC}
- copr-cli installed
- Valid COPR username
- COPR project exists

${YELLOW}Example:${NC}
  $0 myusername     # Upload as myusername

${YELLOW}After Upload:${NC}
sudo dnf copr enable myusername/fedora-pm
sudo dnf install fedora-pm

EOF
}

case "${1:-}" in
    --help|-h)
        show_help
        ;;
    *)
        check_deps
        upload_to_copr "$1"
        ;;
esac