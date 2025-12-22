# Fedora Package Manager

A modern, user-friendly package manager for Fedora Linux that provides both CLI and GUI interfaces for managing packages, kernels, drivers, and gaming setups.

## Features

### Package Management
- **Install packages** - Easy package installation
- **Remove packages** - Clean package removal
- **Update system** - Update all packages or specific ones
- **Search packages** - Search the package repository
- **Package information** - Get detailed info about packages
- **List packages** - List installed or available packages
- **Clean cache** - Clean package cache and metadata
- **History tracking** - Track all package management operations

### Advanced Features
- **Kernel management** - Manage Linux kernels (list, install, remove, info)
- **CachyOS kernels** - Install optimized gaming kernels with BORE scheduler
- **Driver management** - Manage GPU drivers with Nvidia support (detect, install, remove, status)
- **Gaming meta package** - One-click installation of complete gaming setup (Steam, Lutris, Wine, GameMode, MangoHud, DXVK, etc.)

### User Interface
- **CLI interface** - Fast command-line interface for power users
- **Modern Qt GUI** - Beautiful graphical interface built with PySide6
- **Quick install buttons** - One-click installation of gaming packages

## Installation

### Option 1: Install from RPM (Recommended)

Build and install the RPM package:

```bash
# Install build dependencies
sudo dnf install rpm-build rpmdevtools python3-pyside6

# Create source tarball
tar -czf ~/rpmbuild/SOURCES/fedora-pm-1.0.0.tar.gz \
    --exclude='rpmbuild' --exclude='.git' \
    fedora-pm.py fedora-pm-gui.py fedora-pm.desktop \
    README.md requirements.txt install.sh

# Copy spec file
cp rpmbuild/SPECS/fedora-pm.spec ~/rpmbuild/SPECS/

# Build the RPM
cd ~/rpmbuild/SPECS
rpmbuild -ba fedora-pm.spec

# Install the built RPM
sudo dnf install ~/rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

This installs both the CLI (`fedora-pm`) and GUI (`fedora-pm-gui`) versions. The GUI will appear in your applications menu as "Fedora Package Manager".

### Option 2: Manual Installation

1. Make the scripts executable:
```bash
chmod +x fedora-pm.py fedora-pm-gui.py
```

2. Optionally, create symlinks for easier access:
```bash
sudo ln -s $(pwd)/fedora-pm.py /usr/local/bin/fedora-pm
sudo ln -s $(pwd)/fedora-pm-gui.py /usr/local/bin/fedora-pm-gui
```

Or add them to your PATH by copying:
```bash
sudo cp fedora-pm.py /usr/local/bin/fedora-pm
sudo cp fedora-pm-gui.py /usr/local/bin/fedora-pm-gui
sudo chmod +x /usr/local/bin/fedora-pm /usr/local/bin/fedora-pm-gui
```

## Requirements

### System Requirements
- Python 3.6+
- Fedora Linux with `dnf` and `rpm` installed
- sudo access for package operations

### GUI Requirements
- **PySide6** (Qt for Python) - For the graphical interface
  ```bash
  sudo dnf install python3-pyside6
  ```

### Optional Requirements
- **RPM Fusion repositories** - Required for some features (Nvidia drivers, Steam, etc.)
  ```bash
  sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
  sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
  ```

## Usage

### GUI Version

Launch the graphical interface:

```bash
fedora-pm-gui
```

Or find "Fedora Package Manager" in your applications menu.

The GUI provides:
- **Command selector** - Choose from install, remove, update, search, info, list, clean
- **Quick Install** - One-click installation of Fedora Gaming Meta package
- **Auto-confirm option** - Skip confirmation prompts for batch operations
- **Real-time output** - See command output in a terminal-style window
- **Modern interface** - Beautiful Qt-based UI with customizable themes

**Quick Install Gaming Meta:**
Click the "🎮 Install Gaming Meta Package" button in the GUI to install a complete gaming setup including Steam, Lutris, Wine, GameMode, MangoHud, DXVK, and more. The GUI will automatically:
- Check and enable RPM Fusion repositories if needed
- Build the gaming meta package if not already built
- Install all gaming packages with a single click

### CLI Version

### Install packages
```bash
fedora-pm install vim git curl
fedora-pm install vim -y  # Auto-confirm
```

### Remove packages
```bash
fedora-pm remove old-package
fedora-pm remove package1 package2 -y
```

### Update system
```bash
fedora-pm update          # Update all packages
fedora-pm update vim git  # Update specific packages
fedora-pm update -y       # Auto-confirm
```

### Search for packages
```bash
fedora-pm search python
fedora-pm search editor
```

### Get package information
```bash
fedora-pm info vim
fedora-pm info git
```

### List packages
```bash
fedora-pm list installed           # List all installed packages
fedora-pm list installed python    # Filter installed packages
fedora-pm list available            # List available packages
fedora-pm list available editor     # Filter available packages
```

### Clean cache
```bash
fedora-pm clean                     # Clean everything
fedora-pm clean --no-cache          # Clean only metadata
fedora-pm clean --no-metadata       # Clean only cache
```

### View history
```bash
fedora-pm history           # Show last 10 operations
fedora-pm history -n 20     # Show last 20 operations
```

### Kernel management
```bash
fedora-pm kernel current              # Show current running kernel
fedora-pm kernel list                 # List installed kernels
fedora-pm kernel list --available     # List available kernels
fedora-pm kernel install              # Install latest kernel
fedora-pm kernel install 6.5.0       # Install specific kernel version
fedora-pm kernel remove 6.4.0        # Remove specific kernel version
fedora-pm kernel remove-old           # Remove old kernels (keeps 2 newest)
fedora-pm kernel remove-old --keep 3  # Keep 3 newest kernels
fedora-pm kernel info                 # Show current kernel info
fedora-pm kernel info 6.5.0          # Show specific kernel info
```

### CachyOS kernel management
```bash
fedora-pm kernel cachyos list         # List available CachyOS kernels
fedora-pm kernel cachyos check        # Check CachyOS repository status
fedora-pm kernel cachyos enable gcc   # Enable GCC-built kernel repository
fedora-pm kernel cachyos enable lto   # Enable LLVM-ThinLTO kernel repository
fedora-pm kernel cachyos enable both  # Enable both repositories
fedora-pm kernel cachyos check-cpu    # Check CPU instruction set support
fedora-pm kernel cachyos install      # Install default CachyOS kernel (GCC)
fedora-pm kernel cachyos install default --build lto  # Install default (LTO)
fedora-pm kernel cachyos install lts  # Install LTS kernel with BORE scheduler
fedora-pm kernel cachyos install rt   # Install real-time kernel
fedora-pm kernel cachyos install server  # Install server kernel (300Hz)
```

**CachyOS Kernel Types:**
- **default**: Standard optimized kernel with BORE scheduler
- **lts**: Long-Term Support kernel with BORE scheduler (requires x86-64-v2)
- **rt**: Real-time kernel with BORE scheduler
- **server**: Server kernel with 300Hz and EEVDF scheduler (requires x86-64-v2)

**Build Types:**
- **gcc**: GCC-built kernels (default)
- **lto**: LLVM-ThinLTO built kernels (better optimizations)

**Note:** Most CachyOS kernels require x86-64-v3 CPU support. LTS and Server kernels require x86-64-v2. Use `check-cpu` to verify your CPU support.

### Driver management
```bash
fedora-pm driver status              # Show status of all detected GPUs
fedora-pm driver detect              # Detect installed GPU hardware
fedora-pm driver check               # Check Nvidia driver status
fedora-pm driver list nvidia        # List available Nvidia drivers
fedora-pm driver install nvidia      # Install latest Nvidia drivers
fedora-pm driver install nvidia --cuda  # Install Nvidia drivers with CUDA
fedora-pm driver install nvidia --version 535  # Install specific driver version
fedora-pm driver remove nvidia       # Remove Nvidia drivers
fedora-pm driver cuda install        # Install CUDA toolkit
```

**Note:** Nvidia drivers require RPM Fusion repositories. If not enabled, the tool will provide instructions. You can enable them manually:
```bash
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
```

### Gaming Meta Package

Install a complete gaming setup with one command:

**Via GUI:**
- Click the "🎮 Install Gaming Meta Package" button in the GUI

**Via CLI:**
```bash
# Build the gaming meta package
./build-gaming-meta.sh

# Install it
sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm
```

**What's included:**
- Steam - Digital game distribution platform
- Lutris - Open gaming platform for Linux
- Wine + Winetricks - Windows compatibility layer
- GameMode - Automatic performance optimization
- MangoHud - FPS/temperature overlay
- DXVK/VKD3D - DirectX to Vulkan translation layers
- Vulkan - Modern graphics API
- Gaming fonts - Unicode and emoji support
- Controller tools - Gamepad configuration utilities
- PipeWire - Low-latency audio stack

See `fedora-gaming-meta-README.md` and `GAMING_META_QUICKSTART.md` for detailed information.

## Configuration

Configuration is stored in `~/.fedora-pm/config.json`. You can edit this file to customize behavior:

```json
{
  "auto_clean": false,
  "parallel_downloads": true,
  "fastest_mirror": true,
  "color_output": true,
  "history_file": "~/.fedora-pm/history.json"
}
```

## History

All package management operations are logged to `~/.fedora-pm/history.json` for tracking purposes.

## Examples

### Basic Package Management
```bash
# Install development tools
fedora-pm install git vim python3-pip nodejs

# Search for text editors
fedora-pm search editor

# Update system
fedora-pm update -y

# Check what's installed
fedora-pm list installed | grep python

# Clean up after updates
fedora-pm clean

# View recent operations
fedora-pm history
```

### Kernel Management
```bash
# List kernels
fedora-pm kernel list
fedora-pm kernel install
fedora-pm kernel remove-old --keep 2

# CachyOS kernels
fedora-pm kernel cachyos check-cpu
fedora-pm kernel cachyos enable gcc
fedora-pm kernel cachyos install lts
```

### Driver Management
```bash
# Check driver status
fedora-pm driver status
fedora-pm driver detect

# Install Nvidia drivers
fedora-pm driver install nvidia --cuda
fedora-pm driver check
```

### Gaming Setup
```bash
# Install complete gaming setup (via CLI)
./build-gaming-meta.sh
sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm

# Or use the GUI - click "Install Gaming Meta Package" button
fedora-pm-gui
```

## How it works

This package manager is a wrapper around Fedora's native `dnf` and `rpm` tools, providing:
- A cleaner, more intuitive interface (both CLI and GUI)
- Operation history tracking
- Better output formatting
- Configuration management
- One-click gaming setup

### Architecture

**CLI Interface:**
- Direct Python wrapper around system tools
- Fast and scriptable
- Full feature parity with GUI

**GUI Interface:**
- Built with PySide6 (Qt for Python)
- Modern, customizable interface
- Wraps CLI commands for user-friendly operation
- Includes quick install buttons for common tasks

### Under the Hood

The tool uses:
- `dnf` for package installation, removal, updates, and searching
- `rpm` for querying installed packages and package information
- `uname` for kernel version detection
- `lspci` for GPU hardware detection
- `nvidia-smi` for Nvidia driver status checking
- `dnf copr` for managing COPR repositories (CachyOS kernels)
- Kernel management integrates with dnf/rpm for kernel package operations
- Driver management uses RPM Fusion repositories for Nvidia drivers (akmod-nvidia)
- CachyOS kernels are available through COPR repositories (bieszczaders/kernel-cachyos)
- Gaming meta package provides RPM-based meta-package for complete gaming setup

## Customization

### GUI Appearance

The GUI supports extensive customization through Qt stylesheets. See `GUI_CUSTOMIZATION.md` for:
- Color themes (light, dark, custom)
- Font customization
- Button styles
- Window layout adjustments
- And more!

### Gaming Meta Package

Customize the gaming meta package by editing `fedora-gaming-meta.spec` to add or remove packages. See `fedora-gaming-meta-README.md` for details.

## Related Files

- `fedora-pm.py` - Main CLI script
- `fedora-pm-gui.py` - Qt GUI interface
- `fedora-pm.desktop` - Desktop entry file
- `fedora-gaming-meta.spec` - Gaming meta package spec file
- `build-gaming-meta.sh` - Build script for gaming meta package
- `GUI_CUSTOMIZATION.md` - Guide for customizing GUI appearance
- `fedora-gaming-meta-README.md` - Detailed gaming meta package documentation
- `GAMING_META_QUICKSTART.md` - Quick start guide for gaming setup

## License

This project is open source and available for use and modification.

