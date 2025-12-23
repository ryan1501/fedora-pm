#!/bin/bash
# Install all dependencies from fedora-gaming-meta.spec

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Check if repositories are enabled
echo "Checking repository status..."
RPMFUSION_FREE_ENABLED=$(dnf repolist enabled 2>/dev/null | grep -i "rpmfusion.*free" | wc -l)
RPMFUSION_NONFREE_ENABLED=$(dnf repolist enabled 2>/dev/null | grep -i "rpmfusion.*nonfree" | wc -l)

if [ "$RPMFUSION_FREE_ENABLED" -eq 0 ] || [ "$RPMFUSION_NONFREE_ENABLED" -eq 0 ]; then
    echo "⚠ Warning: RPM Fusion repositories may not be fully enabled."
    echo ""
    echo "Some packages require RPM Fusion repositories."
    echo "Run this script first to enable them:"
    echo "  ./enable-repos.sh"
    echo ""
    read -p "Continue anyway? (y/N): " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "Installation cancelled. Please enable repositories first."
        exit 1
    fi
fi

# Extract all Requires: packages from the spec file
PACKAGES=$(grep -E "^Requires:" fedora-gaming-meta.spec | sed 's/Requires: //' | tr '\n' ' ')

echo "=========================================="
echo "Installing dependencies from fedora-gaming-meta.spec"
echo "=========================================="
echo "Packages to install:"
echo "$PACKAGES"
echo ""

sudo dnf install -y $PACKAGES

if [ $? -eq 0 ]; then
    echo ""
    echo "✓ Dependencies installed successfully!"
else
    echo ""
    echo "✗ Some packages failed to install."
    echo "  Make sure RPM Fusion repositories are enabled:"
    echo "  ./enable-repos.sh"
    exit 1
fi

