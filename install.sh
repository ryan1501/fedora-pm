#!/bin/bash
# Installation script for Fedora Package Manager

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/usr/local/bin"
SCRIPT_NAME="fedora-pm"

echo "Installing Fedora Package Manager..."

# Check if running as root or with sudo
if [ "$EUID" -ne 0 ]; then 
    echo "This script needs sudo privileges to install system-wide."
    echo "Please run: sudo $0"
    exit 1
fi

# Copy script to /usr/local/bin
cp "$SCRIPT_DIR/fedora-pm.py" "$INSTALL_DIR/$SCRIPT_NAME"
chmod +x "$INSTALL_DIR/$SCRIPT_NAME"

echo "✓ Fedora Package Manager installed successfully!"
echo ""
echo "You can now use it from anywhere with:"
echo "  $SCRIPT_NAME --help"
echo ""
echo "Example usage:"
echo "  $SCRIPT_NAME install vim git"
echo "  $SCRIPT_NAME search python"
echo "  $SCRIPT_NAME update"

