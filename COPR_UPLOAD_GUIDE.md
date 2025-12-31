# COPR Upload Guide for fedora-pm

## Overview

COPR (Cool Other Package Repo) is Fedora's community repository system for hosting packages. This guide shows how to upload fedora-pm to COPR for distribution to Fedora users.

## Prerequisites

### Required Tools
```bash
# Install COPR tools
sudo dnf install copr-cli
sudo dnf install fedpkg-packager

# Verify installation
copr --version
```

### Fedora Account
- Register at: https://copr.fedorainfra.org/
- Note your COPR username (needed for API access)

## Quick Upload Process

### Step 1: Create Source Tarball
```bash
# Create release tarball
tar -czf fedora-pm-1.1.0.tar.gz \
  --exclude-vcs \
  --exclude='target*' \
  --exclude='rpmbuild*' \
  --exclude='.fingerprint*' \
  --exclude='*.tmp' \
  .
```

### Step 2: Upload via Web Interface
1. Visit: https://copr.fedorainfra.org/coprs/yourusername/fedora-pm/new_build/
2. Upload: `fedora-pm-1.1.0.tar.gz`
3. Configure:
   - **Build Method**: `mock` (Rust packages)
   - **Fedora Versions**: Multiple (39, 40, 41, 42, 43, Rawhide)
   - **Webhook**: (optional, for CI integration)

### Step 3: Monitor Build
```bash
# Watch build progress
copr watch-build fedora-pm

# View build logs
copr get-build fedora-pm 123  # Replace with build ID
```

## Alternative: CLI Upload

### Configure and Upload
```bash
# Using fedpkg-packager
fedpkg-packager --build fedora-pm.spec

# Or direct copr upload
copr upload --name fedora-pm --version 1.1.0 fedora-pm-1.1.0.tar.gz
```

## User Installation

Once uploaded to COPR:
```bash
# Add repository
sudo dnf copr enable yourusername/fedora-pm

# Install
sudo dnf install fedora-pm
```

## Files Provided

- `fedora-pm.spec` - RPM spec file for packaging
- Proper source tarball structure
- GitHub integration documentation

---

**Status**: 🚀 Ready for COPR upload  
**Version**: 1.1.0  
**Recommended**: Web upload for initial setup