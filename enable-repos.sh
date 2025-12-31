#!/usr/bin/env bash

# Enable required repositories for fedora-pm
# Use unified install.sh for all operations

set -euo pipefail
trap 'echo "Error on line $LINENO" >&2; exit 1' ERR

# Use unified install.sh for all operations
echo "Note: Use './install.sh --help' for comprehensive installation options."