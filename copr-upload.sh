#!/bin/bash

# Simple COPR upload with multiple authentication methods
# Usage: ./copr-upload.sh [options]

set -e

# Configuration
PROJECT_NAME="fedora-pm"
VERSION="1.1.0"
USERNAME="uncodedchristiangamer"
CHANGELOG="GitHub-based self-update integration with smart GitHub API integration and fallback mechanisms"

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
create_tarball() {
    info "Creating source tarball..."
    TAR_FILE="${PROJECT_NAME}-${VERSION}.tar.gz"
    
    tar -czf "${TAR_FILE}" \
        --exclude-vcs \
        --exclude='target*' \
        --exclude='rpmbuild*' \
        --exclude='.fingerprint*' \
        --exclude='*.tmp' \
        .
    
    success "Created: ${TAR_FILE}"
    echo "${TAR_FILE}"
}

# Upload methods
upload_with_token() {
    local token="$1"
    info "Uploading with API token..."
    
    copr upload --name "${PROJECT_NAME}" --version "${VERSION}" --changelog "${CHANGELOG}" "${PROJECT_NAME}-${VERSION}.tar.gz"
    
    if [ $? -eq 0 ]; then
        success "Upload successful via token!"
    else
        error "Upload failed via token"
    fi
}

upload_with_username() {
    info "Uploading with username parameter..."
    
    copr --username "${USERNAME}" upload --name "${PROJECT_NAME}" --version "${VERSION}" --changelog "${CHANGELOG}" "${PROJECT_NAME}-${VERSION}.tar.gz"
    
    if [ $? -eq 0 ]; then
        success "Upload successful via username!"
    else
        error "Upload failed via username"
    fi
}

web_upload_instructions() {
    TAR_FILE=$(create_tarball)
    
    cat << EOF
${BLUE}Web Upload Instructions:${NC}

1. Visit: ${YELLOW}https://copr.fedorainfracloud.org/coprs/${USERNAME}/${PROJECT_NAME}/new_build/${NC}
2. Upload: ${GREEN}${TAR_FILE}${NC}
3. Configure:
   - Build Method: mock
   - Fedora Versions: 39, 40, 41, 42, 43, Rawhide
   - Changelog: ${CHANGELOG}

4. Monitor builds on the web interface
EOF
}

show_help() {
    cat << EOF
${BLUE}COPR Upload Script${NC}

${YELLOW}Usage:${NC} $0 [METHOD] [OPTIONS]

${BLUE}Methods:${NC}
  web               Show web upload instructions
  token <TOKEN>    Upload using API token
  username          Upload using username parameter

${BLUE}Examples:${NC}
  $0 web                                    # Show web upload instructions
  $0 token abc123token                    # Upload with API token
  $0 username                               # Upload with username flag

${BLUE}Token Generation:${NC}
  Visit: https://copr.fedorainfracloud.org/api/token/
  Click "Generate new token" and copy it

${BLUE}Web Upload:${NC}
  Visit: https://copr.fedorainfracloud.org/coprs/uncodedchristiangamer/fedora-pm/new_build/
EOF
}

# Main menu
case "${1:-}" in
    --help|-h)
        show_help
        ;;
    web)
        web_upload_instructions
        ;;
    token)
        create_tarball
        if [ -z "$2" ]; then
            error "Token required. Usage: $0 token <your-api-token>"
        fi
        upload_with_token "$2"
        ;;
    username)
        create_tarball
        upload_with_username
        ;;
    *)
        echo "No method specified. Choose one:"
        echo "  $0 web        # Web upload instructions"
        echo "  $0 token TOKEN # Upload with API token"
        echo "  $0 username   # Upload with username flag"
        echo "  $0 --help     # Show this help"
        exit 1
        ;;
esac