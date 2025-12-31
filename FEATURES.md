# Fedora-PM Features Guide

This document provides detailed information about all features available in fedora-pm.

## Table of Contents

1. [Core Package Management](#core-package-management)
2. [Dependency Management](#dependency-management)
3. [Rollback & History](#rollback--history)
4. [Package Groups](#package-groups)
5. [System Health Check](#system-health-check)
6. [Flatpak Integration](#flatpak-integration)
7. [Backup & Restore](#backup--restore)
8. [Repository Management](#repository-management)
9. [Security Management](#security-management)
10. [Download & Offline Installation](#download--offline-installation)
11. [Changelog Viewer](#changelog-viewer)
12. [Disk Space Analysis](#disk-space-analysis)
13. [Kernel Management](#kernel-management)
14. [Driver Management](#driver-management)
15. [Gaming Meta Package](#gaming-meta-package)

---

## Core Package Management

### Install Packages
```bash
fedora-pm install <package1> <package2> ...
fedora-pm install vim git -y  # Skip confirmation
```

### Remove Packages
```bash
fedora-pm remove <package1> <package2> ...
fedora-pm remove old-package -y
```

### Update System
```bash
fedora-pm update              # Update all packages
fedora-pm update vim git      # Update specific packages
fedora-pm update -y           # Skip confirmation
```

### Search Packages
```bash
fedora-pm search <query>
fedora-pm search python
```

### Package Information
```bash
fedora-pm info <package>
fedora-pm info vim
```

### List Packages
```bash
fedora-pm list installed           # List installed packages
fedora-pm list installed python    # Filter installed packages
fedora-pm list available           # List available packages
fedora-pm list available editor    # Filter available packages
```

### Clean Cache
```bash
fedora-pm clean                    # Clean everything
fedora-pm clean --no-cache         # Clean only metadata
fedora-pm clean --no-metadata      # Clean only cache
```

---

## Dependency Management

View package dependencies and reverse dependencies.

### Dependency Tree
```bash
fedora-pm deps <package> --tree
fedora-pm deps vim --tree
```

Shows a hierarchical tree of all dependencies for a package.

### Reverse Dependencies
```bash
fedora-pm deps <package> --reverse
fedora-pm deps vim --reverse
```

Shows what packages depend on the specified package.

**Use Cases:**
- Understanding package relationships before removal
- Troubleshooting dependency issues
- Planning system changes

---

## Rollback & History

### View History
```bash
fedora-pm history              # Show last 10 operations
fedora-pm history -n 20        # Show last 20 operations
```

### Rollback Operations
```bash
fedora-pm rollback             # Undo last operation
fedora-pm rollback --id 5      # Rollback specific operation
fedora-pm rollback --yes       # Skip confirmation
```

**Supported Operations:**
- ✅ Install → Removes installed packages
- ✅ Remove → Reinstalls removed packages
- ⚠️ Update → Shows warning (use dnf history for downgrades)

**Use Cases:**
- Undo accidental installations
- Recover from package removals
- Quick system state restoration

---

## Package Groups

Manage collections of related packages.

### List Groups
```bash
fedora-pm group list
```

### Group Information
```bash
fedora-pm group info "Development Tools"
```

### Install Group
```bash
fedora-pm group install "Development Tools"
fedora-pm group install "Development Tools" -y
```

### Remove Group
```bash
fedora-pm group remove "Development Tools"
fedora-pm group remove "Development Tools" -y
```

**Popular Groups:**
- Development Tools
- C Development Tools and Libraries
- System Tools
- Virtualization
- Container Management

---

## System Health Check

Run comprehensive system diagnostics.

```bash
fedora-pm doctor
```

**Checks Performed:**
1. ✅ Broken dependencies
2. ✅ Orphaned packages
3. ✅ Disk space usage
4. ✅ Repository status
5. ✅ Duplicate packages
6. ✅ Security updates

**Output:**
- Green ✓ - No issues
- Yellow ⚠ - Warning
- Red ✗ - Critical issue

**Use Cases:**
- Regular system maintenance
- Troubleshooting package issues
- Pre-upgrade checks
- System health monitoring

---

## Flatpak Integration

Manage Flatpak applications alongside DNF packages.

### Setup Flathub
```bash
fedora-pm flatpak setup-flathub
```

### Search Flatpaks
```bash
fedora-pm flatpak search <query>
fedora-pm flatpak search spotify
```

### Install Flatpak
```bash
fedora-pm flatpak install <app-id>
fedora-pm flatpak install com.spotify.Client -y
```

### List Flatpaks
```bash
fedora-pm flatpak list
```

### Update Flatpaks
```bash
fedora-pm flatpak update
fedora-pm flatpak update -y
```

### Remove Flatpak
```bash
fedora-pm flatpak remove <app-id>
fedora-pm flatpak remove com.spotify.Client -y
```

### Flatpak Info
```bash
fedora-pm flatpak info <app-id>
fedora-pm flatpak info com.spotify.Client
```

**Benefits:**
- Unified package management
- Access to latest applications
- Sandboxed applications
- Cross-distribution compatibility

---

## Backup & Restore

Export and import package lists for system migration or backup.

### Export Packages
```bash
fedora-pm export packages.txt
fedora-pm export packages.txt --with-flatpak
```

### Import Packages
```bash
fedora-pm import packages.txt
fedora-pm import packages.txt -y
```

**Use Cases:**
- System migration
- Backup before major changes
- Replicate setups across machines
- Disaster recovery

**File Format:**
- Plain text, one package per line
- Comments start with #
- Flatpaks marked with # flatpak: prefix

---

## Repository Management

Manage DNF repositories easily.

### List Repositories
```bash
fedora-pm repo list              # Enabled only
fedora-pm repo list --all        # All repositories
```

### Enable Repository
```bash
fedora-pm repo enable <repo-id>
fedora-pm repo enable rpmfusion-free
```

### Disable Repository
```bash
fedora-pm repo disable <repo-id>
fedora-pm repo disable updates-testing
```

### Add Repository
```bash
fedora-pm repo add <name> <url>
fedora-pm repo add myrepo https://example.com/repo.repo
```

### Remove Repository
```bash
fedora-pm repo remove <repo-id>
```

### Repository Info
```bash
fedora-pm repo info <repo-id>
fedora-pm repo info fedora
```

### Refresh Metadata
```bash
fedora-pm repo refresh
```

**Common Repositories:**
- rpmfusion-free
- rpmfusion-nonfree
- updates-testing
- fedora-cisco-openh264

---

## Security Management

Manage security updates and perform security audits.

### Check Security Updates
```bash
fedora-pm security check
```

### List Security Updates
```bash
fedora-pm security list
fedora-pm security list --severity critical
fedora-pm security list --severity important
```

### Install Security Updates
```bash
fedora-pm security update
fedora-pm security update -y
```

### Security Audit
```bash
fedora-pm security audit
```

Shows:
- Security updates available
- Critical updates
- Important updates

### Check CVE
```bash
fedora-pm security cve <CVE-ID>
fedora-pm security cve CVE-2024-1234
```

### Advisory Information
```bash
fedora-pm security info <advisory-id>
fedora-pm security info FEDORA-2024-123
```

**Best Practices:**
- Run `security audit` weekly
- Install security updates promptly
- Monitor critical CVEs
- Review advisories before updating

---

## Download & Offline Installation

Download packages for offline installation.

### Download Packages
```bash
fedora-pm download <package1> <package2> ...
fedora-pm download vim git
```

### Download to Directory
```bash
fedora-pm download <packages> --dest <directory>
fedora-pm download vim --dest /tmp/packages
```

### Download with Dependencies
```bash
fedora-pm download <packages> --with-deps
fedora-pm download firefox --with-deps --dest /media/usb
```

### Install Offline
```bash
fedora-pm install-offline <rpm-files>
fedora-pm install-offline *.rpm -y
```

**Use Cases:**
- Offline installations
- Air-gapped systems
- Slow internet connections
- Package caching

---

## Changelog Viewer

View package changelogs and update information.

### View Changelog
```bash
fedora-pm changelog <package>
fedora-pm changelog vim
```

### Recent Changelog Entries
```bash
fedora-pm changelog <package> -n <count>
fedora-pm changelog vim -n 5
```

### What's New
```bash
fedora-pm whatsnew
```

Shows changelogs for all pending updates.

**Use Cases:**
- Review changes before updating
- Understand bug fixes
- Check for breaking changes
- Security update details

---

## Disk Space Analysis

Analyze package disk usage and manage space.

### Full Analysis
```bash
fedora-pm size --analyze
```

Shows:
- Total installed package size
- DNF cache size
- Root filesystem usage
- Orphaned packages count
- Recommendations

### Top Packages
```bash
fedora-pm size --top <count>
fedora-pm size --top 20
```

Lists largest installed packages.

### Total Size
```bash
fedora-pm size --total
```

Shows total size of all installed packages.

### Clean Orphaned Packages
```bash
fedora-pm clean-orphans
fedora-pm clean-orphans -y
```

**Use Cases:**
- Free up disk space
- Identify large packages
- Clean up after uninstalls
- System maintenance

---

## Kernel Management

Manage Linux kernels including CachyOS optimized kernels.

### Current Kernel
```bash
fedora-pm kernel current
```

### List Kernels
```bash
fedora-pm kernel list              # Installed kernels
fedora-pm kernel list --available  # Available kernels
```

### Install Kernel
```bash
fedora-pm kernel install           # Latest kernel
fedora-pm kernel install 6.5.0     # Specific version
```

### Remove Kernel
```bash
fedora-pm kernel remove 6.4.0
fedora-pm kernel remove 6.4.0 -y
```

### Remove Old Kernels
```bash
fedora-pm kernel remove-old           # Keep 2 newest
fedora-pm kernel remove-old --keep 3  # Keep 3 newest
```

### Kernel Info
```bash
fedora-pm kernel info              # Current kernel
fedora-pm kernel info 6.5.0        # Specific kernel
```

### CachyOS Kernels

Optimized gaming kernels with BORE scheduler.

#### Check CPU Support
```bash
fedora-pm kernel cachyos check-cpu
```

#### Enable Repository
```bash
fedora-pm kernel cachyos enable gcc   # GCC-built kernels
fedora-pm kernel cachyos enable lto   # LLVM-ThinLTO kernels
fedora-pm kernel cachyos enable both  # Both repositories
```

#### List CachyOS Kernels
```bash
fedora-pm kernel cachyos list
```

#### Install CachyOS Kernel
```bash
fedora-pm kernel cachyos install              # Default kernel
fedora-pm kernel cachyos install lts          # LTS kernel
fedora-pm kernel cachyos install rt           # Real-time kernel
fedora-pm kernel cachyos install server       # Server kernel
fedora-pm kernel cachyos install default --build lto  # LTO build
```

**Kernel Types:**
- **default**: Standard optimized kernel with BORE scheduler
- **lts**: Long-Term Support with BORE (requires x86-64-v2)
- **rt**: Real-time kernel with BORE
- **server**: Server kernel with 300Hz and EEVDF (requires x86-64-v2)

---

## Driver Management

Manage GPU drivers, especially Nvidia.

### Driver Status
```bash
fedora-pm driver status
```

### Detect Hardware
```bash
fedora-pm driver detect
```

### Nvidia Drivers

#### List Available Drivers
```bash
fedora-pm driver list-nvidia
```

#### Install Nvidia Drivers
```bash
fedora-pm driver install-nvidia              # Latest drivers
fedora-pm driver install-nvidia --cuda       # With CUDA
fedora-pm driver install-nvidia --version 535  # Specific version
```

#### Check Nvidia Status
```bash
fedora-pm driver check-nvidia
```

#### Remove Nvidia Drivers
```bash
fedora-pm driver remove-nvidia
fedora-pm driver remove-nvidia -y
```

#### Install CUDA
```bash
fedora-pm driver install-cuda
fedora-pm driver install-cuda -y
```

**Requirements:**
- RPM Fusion repositories must be enabled
- Secure Boot may need to be disabled
- Reboot required after installation

---

## Gaming Meta Package

One-click installation of complete gaming setup.

### Via CLI
```bash
./enable-repos.sh
fedora-pm gaming install
```

### Via GUI
Click the "🎮 Install Gaming Meta Package" button

### What's Included

**Game Platforms:**
- Steam
- Lutris
- Heroic Games Launcher

**Compatibility Layers:**
- Wine
- Winetricks
- Proton (via Steam)

**Performance Tools:**
- GameMode
- MangoHud
- CoreCtrl

**Graphics:**
- DXVK
- VKD3D
- Vulkan

**Audio:**
- PipeWire
- Low-latency audio stack

**Utilities:**
- Controller tools
- Gaming fonts
- Unicode/emoji support

**Use Cases:**
- New gaming setup
- Quick gaming environment
- Consistent gaming configuration
- Steam Deck-like experience

---

## Verbose Logging

Control output verbosity for debugging.

```bash
fedora-pm install vim              # Normal output
fedora-pm -v install vim           # Verbose (warnings)
fedora-pm -vv install vim          # Very verbose (info)
fedora-pm -vvv install vim         # Debug output
fedora-pm --quiet install vim      # Minimal output
```

**Log Levels:**
- Default: Errors only
- `-v`: Warnings
- `-vv`: Info
- `-vvv`: Debug

---

## Tips & Best Practices

### Regular Maintenance
```bash
# Weekly routine
fedora-pm doctor
fedora-pm security audit
fedora-pm update -y
fedora-pm clean
```

### Before Major Updates
```bash
fedora-pm export backup-$(date +%Y%m%d).txt --with-flatpak
fedora-pm size --analyze
fedora-pm whatsnew
```

### System Migration
```bash
# On old system
fedora-pm export my-system.txt --with-flatpak

# On new system
fedora-pm import my-system.txt -y
```

### Troubleshooting
```bash
fedora-pm doctor
fedora-pm deps <problematic-package> --tree
fedora-pm -vv install <package>
```

---

## Getting Help

```bash
fedora-pm --help                    # General help
fedora-pm <command> --help          # Command-specific help
fedora-pm kernel --help             # Subcommand help
fedora-pm kernel cachyos --help     # Nested subcommand help
```

For more information, see the main [README.md](README.md).
