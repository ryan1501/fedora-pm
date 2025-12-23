#!/bin/bash
# Check which repositories provide each package from the spec file

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f "fedora-gaming-meta.spec" ]; then
    echo "Error: fedora-gaming-meta.spec not found"
    exit 1
fi

echo "=========================================="
echo "Package Repository Mapping"
echo "=========================================="
echo ""

# Extract packages from spec file
PACKAGES=$(grep -E "^Requires:" fedora-gaming-meta.spec | sed 's/Requires: //')

# Group packages by repository
declare -A REPO_PACKAGES

echo "Checking package availability..."
echo ""

for pkg in $PACKAGES; do
    # Get repository info for each package
    REPO_INFO=$(dnf repoquery --available --qf "%{repoid}" "$pkg" 2>/dev/null | head -1)
    
    if [ -n "$REPO_INFO" ]; then
        if [ -z "${REPO_PACKAGES[$REPO_INFO]}" ]; then
            REPO_PACKAGES[$REPO_INFO]="$pkg"
        else
            REPO_PACKAGES[$REPO_INFO]="${REPO_PACKAGES[$REPO_INFO]} $pkg"
        fi
        printf "%-30s -> %s\n" "$pkg" "$REPO_INFO"
    else
        printf "%-30s -> NOT FOUND\n" "$pkg"
    fi
done

echo ""
echo "=========================================="
echo "Packages by Repository"
echo "=========================================="
echo ""

for repo in "${!REPO_PACKAGES[@]}"; do
    echo "Repository: $repo"
    echo "  Packages:"
    for pkg in ${REPO_PACKAGES[$repo]}; do
        echo "    - $pkg"
    done
    echo ""
done

echo "=========================================="
echo "Repository Status"
echo "=========================================="
echo ""
dnf repolist enabled | grep -E "repo id|rpmfusion|fedora"

