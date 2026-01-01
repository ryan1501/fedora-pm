# Fedora Package Manager

A modern, high-performance package manager for Fedora Linux, completely rewritten in native Rust with both CLI and native GUI interfaces. Built for speed, security, and reliability.

## 🚀 Quick Start

**Install both CLI and GUI (recommended):**
```bash
./install.sh --both
```

**Installation Options:**
```bash
./install.sh --cli        # CLI only (lightweight)
./install.sh --gui        # GUI only
./install.sh --both       # CLI + GUI (complete setup)
./install.sh --user --both # Install to user directory
```

## ✨ Key Features

### Package Management
- **Fast Operations** - Install, remove, update, search, info, list, clean
- **Smart Search** - Find packages instantly with intelligent filtering
- **Batch Operations** - Handle multiple packages efficiently
- **History Tracking** - Complete operation history with rollback capabilities
- **Dependency Management** - Visualize dependencies and reverse dependencies
- **Package Groups** - Manage package collections easily

### System Management
- **Health Diagnostics** - Comprehensive system health checks (`doctor`)
- **Security Audits** - Check for vulnerabilities and security updates
- **Disk Analysis** - Analyze package sizes and disk usage
- **Orphan Cleanup** - Find and remove unused packages
- **Repository Management** - Enable, disable, add, and manage repositories

### Advanced Features
- **Kernel Management** - Install, remove, and manage Linux kernels
- **CachyOS Kernels** - Optimized gaming kernels with BORE scheduler
- **Driver Management** - GPU driver management with Nvidia support
- **Gaming Setup** - One-click gaming environment installation
- **Flatpak Integration** - Manage Flatpak applications alongside DNF packages
- **Backup & Restore** - Export/import package lists for system migration
- **Offline Installation** - Download packages for offline installation
- **Changelog Viewer** - View package changelogs before updates
- **Download Manager** - Download packages with dependencies

### GUI Features
- **Native Rust GUI** - Built with `iced` for optimal performance
- **Modern Interface** - Clean, responsive design
- **Real-time Updates** - Live package information and system status
- **Integrated Terminal** - Command output directly in the GUI
- **Quick Actions** - One-click common operations
- **System Monitoring** - Resource usage and system information

## 📦 Installation

### System Requirements
- Fedora Linux with `dnf` and `rpm`
- sudo access for package operations

### Quick Installation

#### Method 1: Unified Installer (Recommended)
```bash
# Install both CLI and GUI
./install.sh --both

# Install CLI only
./install.sh --cli

# Install GUI only
./install.sh --gui
```

#### Method 2: From COPR Repository
```bash
# Enable COPR repository
sudo dnf copr enable uncodedchristiangamer/fedora-pm

# Install CLI and GUI
sudo dnf install fedora-pm fedora-pm-gui
```

#### Method 3: Manual Build
```bash
# Build from source
cargo build --release

# Install CLI
sudo install -m 0755 target/release/fedora-pm /usr/local/bin/fedora-pm

# Install GUI
sudo install -m 0755 target/release/fedora-pm-gui /usr/local/bin/fedora-pm-gui
```

### Installation Features

The unified installer provides:
- ✅ **Automatic Dependencies** - Installs Rust and required packages
- ✅ **Smart Detection** - Finds existing binaries
- ✅ **Flexible Paths** - System-wide, user, or custom installation
- ✅ **Dry Run Mode** - Preview installation before executing
- ✅ **Colored Output** - Professional, easy-to-read feedback
- ✅ **Error Handling** - Graceful fallbacks and clear messages

## 🎯 Usage

### GUI Application

Launch the graphical interface:
```bash
fedora-pm-gui
```

The GUI provides:
- **Dashboard** - System overview and quick actions
- **Package Management** - Install, remove, update packages
- **System Tools** - Health check, disk analysis, security
- **Gaming Center** - Gaming setup and optimization
- **Settings** - Configuration and preferences

### CLI Examples

#### Basic Package Management
```bash
# Install packages
fedora-pm install vim git curl

# Remove packages
fedora-pm remove old-package

# Update system
fedora-pm update

# Search packages
fedora-pm search python

# Package information
fedora-pm info vim

# List packages
fedora-pm list installed
fedora-pm list available editor
```

#### System Health & Maintenance
```bash
# System health check
fedora-pm doctor

# Security audit
fedora-pm security audit

# Disk space analysis
fedora-pm size --analyze

# Clean orphaned packages
fedora-pm clean-orphans
```

#### Advanced Operations
```bash
# Dependency tree
fedora-pm deps vim --tree

# Rollback operations
fedora-pm rollback

# Export packages
fedora-pm export packages.txt --with-flatpak

# Import packages
fedora-pm import packages.txt

# Repository management
fedora-pm repo list
fedora-pm repo enable rpmfusion-free
```

#### Kernel Management
```bash
# List kernels
fedora-pm kernel list

# Install latest kernel
fedora-pm kernel install

# Install CachyOS kernel
fedora-pm kernel cachyos install

# Remove old kernels
fedora-pm kernel remove-old --keep 2
```

#### Gaming Setup
```bash
# Install gaming meta package
fedora-pm gaming install

# Manage gaming drivers
fedora-pm driver install nvidia --cuda
```

#### Flatpak Management
```bash
# Setup Flathub
fedora-pm flatpak setup-flathub

# Install Flatpak apps
fedora-pm flatpak install com.spotify.Client

# Update all Flatpaks
fedora-pm flatpak update
```

## 🔧 Configuration

Configuration is stored in `~/.fedora-pm/config.json`:

```json
{
  "auto_clean": false,
  "parallel_downloads": true,
  "fastest_mirror": true,
  "color_output": true,
  "history_file": "~/.fedora-pm/history.json"
}
```

## 📊 Version Information

**Current Version:** 1.1.0

### Recent Changes
- ✅ **Native Rust GUI** - Migrated from Python to native Rust with `iced`
- ✅ **Self-Update** - Built-in update functionality
- ✅ **Unified Installation** - Single script for all installation methods
- ✅ **Enhanced Performance** - Significant speed improvements
- ✅ **Better Error Handling** - More robust error recovery
- ✅ **Improved Integration** - Better system integration

### Architecture
- **CLI Engine** - High-performance Rust command-line interface
- **Native GUI** - Rust-based graphical interface using `iced`
- **Modular Design** - Clean, maintainable code structure
- **Comprehensive Feature Set** - 70+ commands across all categories

## 🎮 Gaming Support

Complete gaming environment setup with one command:
```bash
fedora-pm gaming install
```

**Includes:**
- Steam, Lutris, Wine
- GameMode, MangoHud, DXVK
- Gaming optimizations and drivers
- Controller support utilities

## 🔒 Security Features

- **Security Audits** - Check for vulnerabilities
- **CVE Monitoring** - Track security advisories
- **Security-Only Updates** - Install critical updates only
- **Advisory Information** - Detailed security bulletins

## 📁 Backup & Migration

Export your system setup:
```bash
fedora-pm export my-backup.txt --with-flatpak
```

Restore on another system:
```bash
fedora-pm import my-backup.txt
```

## 🛠️ Development

### Build from Source
```bash
# Clone repository
git clone https://github.com/ryan1501/fedora-pm
cd fedora-pm

# Build in release mode
cargo build --release

# Run tests
cargo test

# Install
sudo install -m 0755 target/release/fedora-pm /usr/local/bin/fedora-pm
sudo install -m 0755 target/release/fedora-pm-gui /usr/local/bin/fedora-pm-gui
```

### Project Structure
```
src/
├── main.rs          # Main application entry
├── package.rs       # Package operations
├── kernel.rs        # Kernel management
├── driver.rs        # Driver management
├── gaming.rs        # Gaming setup
├── security.rs      # Security features
├── flatpak.rs       # Flatpak integration
├── doctor.rs        # System diagnostics
├── diskspace.rs     # Disk analysis
├── export.rs        # Backup/restore
├── rollback.rs      # Operation rollback
├── deps.rs          # Dependencies
├── groups.rs        # Package groups
├── repo.rs          # Repository management
├── download.rs      # Package downloads
├── changelog.rs     # Changelog viewer
├── history.rs       # Operation history
├── config.rs        # Configuration
├── help.rs          # Help system
└── runner.rs        # Command execution
```

## 📚 Documentation

- **Features Guide** - See `FEATURES.md` for detailed feature documentation
- **Quick Reference** - See `QUICK_REFERENCE.md` for command lookup
- **Gaming Guide** - See `GAMING_META_QUICKSTART.md` for gaming setup
- **Installation** - See `INSTALLATION_UNIFICATION_COMPLETE.md` for detailed install options

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## 📄 License

This project is licensed under the MIT or Apache-2.0 License.

## 🆘 Support

### Getting Help
```bash
fedora-pm --help
fedora-pm <command> --help
```

### Troubleshooting
- Check system requirements
- Ensure sudo access for system operations
- Verify COPR repository is enabled for RPM installation
- Check the installation log for errors

### Bug Reports
Please report issues on the GitHub repository with:
- System information (Fedora version)
- Error messages
- Steps to reproduce
- Expected vs actual behavior

---

## 🎉 Summary

Fedora Package Manager provides:
- 🚀 **Lightning-fast CLI** with 70+ commands
- 🖥️ **Native GUI** built with Rust
- 🎮 **Gaming support** with one-click setup
- 🔒 **Security features** for system protection
- 📊 **System analytics** and health monitoring
- 💾 **Backup & migration** capabilities
- 📦 **Complete package management** solution

**Experience the next generation of Fedora package management!** 🚀

---

*Version 1.1.0 | Built with ❤️ in Rust*