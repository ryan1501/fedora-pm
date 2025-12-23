#!/bin/bash
# Enable required repositories for Fedora Gaming Meta packages

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "Fedora Gaming Meta - Repository Setup"
echo "=========================================="
echo ""

# Get Fedora version
FEDORA_VERSION=$(rpm -E %fedora 2>/dev/null || echo "")
if [ -z "$FEDORA_VERSION" ]; then
    echo "Error: Could not determine Fedora version"
    echo "Please run: rpm -E %fedora"
    exit 1
fi

echo "Detected Fedora version: $FEDORA_VERSION"
echo ""

# Check current repository status
echo "Checking current repository status..."
RPMFUSION_FREE_ENABLED=$(dnf repolist enabled 2>/dev/null | grep -i "rpmfusion.*free" | wc -l)
RPMFUSION_NONFREE_ENABLED=$(dnf repolist enabled 2>/dev/null | grep -i "rpmfusion.*nonfree" | wc -l)

# Enable RPM Fusion Free
if [ "$RPMFUSION_FREE_ENABLED" -eq 0 ]; then
    echo "RPM Fusion Free repository is not enabled."
    echo "Enabling RPM Fusion Free repository..."
    FREE_URL="https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-${FEDORA_VERSION}.noarch.rpm"
    echo "URL: $FREE_URL"
    sudo dnf install -y "$FREE_URL"
    if [ $? -eq 0 ]; then
        echo "✓ RPM Fusion Free repository enabled"
    else
        echo "✗ Failed to enable RPM Fusion Free repository"
        exit 1
    fi
else
    echo "✓ RPM Fusion Free repository is already enabled"
fi

echo ""

# Enable RPM Fusion Nonfree
if [ "$RPMFUSION_NONFREE_ENABLED" -eq 0 ]; then
    echo "RPM Fusion Nonfree repository is not enabled."
    echo "Enabling RPM Fusion Nonfree repository..."
    NONFREE_URL="https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-${FEDORA_VERSION}.noarch.rpm"
    echo "URL: $NONFREE_URL"
    sudo dnf install -y "$NONFREE_URL"
    if [ $? -eq 0 ]; then
        echo "✓ RPM Fusion Nonfree repository enabled"
    else
        echo "✗ Failed to enable RPM Fusion Nonfree repository"
        exit 1
    fi
else
    echo "✓ RPM Fusion Nonfree repository is already enabled"
fi

echo ""
echo "=========================================="
echo "Repository Status"
echo "=========================================="
dnf repolist enabled | grep -E "repo id|rpmfusion|fedora" | head -20

echo ""
echo "=========================================="
echo "Checking Package Availability"
echo "=========================================="

# Extract packages from spec file
if [ -f "fedora-gaming-meta.spec" ]; then
    PACKAGES=$(grep -E "^Requires:" fedora-gaming-meta.spec | sed 's/Requires: //' | tr '\n' ' ')
    
    echo "Checking which repositories provide the required packages..."
    echo ""
    
    MISSING_PACKAGES=()
    FOUND_PACKAGES=()
    
    for pkg in $PACKAGES; do
        # Check if package is available
        REPO_INFO=$(dnf repoquery --available --qf "%{name} from %{repoid}" "$pkg" 2>/dev/null | head -1)
        if [ -n "$REPO_INFO" ]; then
            echo "✓ $REPO_INFO"
            FOUND_PACKAGES+=("$pkg")
        else
            echo "✗ $pkg - NOT FOUND in any enabled repository"
            MISSING_PACKAGES+=("$pkg")
        fi
    done
    
    echo ""
    echo "=========================================="
    echo "Summary"
    echo "=========================================="
    echo "Found packages: ${#FOUND_PACKAGES[@]}"
    echo "Missing packages: ${#MISSING_PACKAGES[@]}"
    
    if [ ${#MISSING_PACKAGES[@]} -gt 0 ]; then
        echo ""
        echo "Missing packages:"
        for pkg in "${MISSING_PACKAGES[@]}"; do
            echo "  - $pkg"
        done
        echo ""
        echo "Note: Some packages may need additional repositories or may not be available for your Fedora version."
    fi
else
    echo "Warning: fedora-gaming-meta.spec not found. Skipping package availability check."
fi

echo ""
echo "=========================================="
echo "Repository setup complete!"
echo "=========================================="
echo ""
echo "You can now install dependencies with:"
echo "  ./install-dependencies.sh"
echo ""
echo "Or build and install the meta-package:"
echo "  ./build-gaming-meta.sh"
echo "  sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm"

