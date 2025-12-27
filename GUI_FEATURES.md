# Fedora-PM GUI Features

The Fedora Package Manager GUI has been completely redesigned with a modern tabbed interface and support for all new CLI features.

## Overview

The GUI is organized into 4 main tabs:
- **📦 Packages** - Core package management
- **🔧 System** - System health, security, and gaming
- **📱 Flatpak** - Flatpak application management
- **⚙️ Advanced** - Repository management, analysis tools, and downloads

## Tab 1: 📦 Packages

### Package Operations
- **Command Selection**: Choose from install, remove, update, search, info, list installed, list available, clean, history
- **Package Input**: Enter package names or arguments
- **Auto-confirm**: Checkbox to automatically confirm operations (-y flag)
- **Run Command**: Execute the selected operation

### Quick Actions
- **⏮️ Rollback Last Operation**: Undo the last package operation
- **💾 Export Packages**: Save installed package list to a file (includes Flatpaks)
- **📥 Import Packages**: Restore packages from a backup file

## Tab 2: 🔧 System

### System Health & Maintenance
- **🏥 Run System Health Check (Doctor)**: Comprehensive system diagnostics
  - Checks broken dependencies
  - Finds orphaned packages
  - Analyzes disk space
  - Verifies repository status
  - Detects duplicate packages
  - Lists security updates

- **💾 Analyze Disk Space**: Full disk space analysis
  - Total package size
  - Cache size
  - Filesystem usage
  - Recommendations

- **🗑️ Clean Orphaned Packages**: Remove packages no longer needed

### Security Management
- **🔒 Run Security Audit**: Complete security assessment
  - Security updates available
  - Critical updates
  - Important updates

- **🛡️ Install Security Updates**: Install only security-related updates

### Gaming Setup
- **🎮 Install Gaming Meta Package**: One-click gaming environment setup
  - Automatically checks and enables RPM Fusion
  - Builds package if needed
  - Installs Steam, Lutris, Wine, GameMode, MangoHud, DXVK, and more

## Tab 3: 📱 Flatpak

### Flatpak Setup
- **🔧 Setup Flathub Repository**: Configure Flathub for Flatpak apps

### Flatpak Operations
- **Search**: Find Flatpak applications
- **Install**: Install Flatpak apps by ID (e.g., com.spotify.Client)
- **📋 List Installed**: Show all installed Flatpaks
- **🔄 Update All**: Update all Flatpak applications

## Tab 4: ⚙️ Advanced

### Repository Management
- **📋 List Repositories**: Show all repositories (enabled and disabled)
- **🔄 Refresh Repository Metadata**: Update repository cache

### Information & Analysis
- **🌳 Dependency Tree**: Visualize package dependencies
  - Enter package name
  - Click to show dependency tree

- **📜 What's New in Updates**: View changelogs for pending updates

- **📊 Show Top 20 Largest Packages**: Identify space-consuming packages

### Download & Offline Tools
- **⬇️ Download**: Download packages with dependencies for offline installation
  - Enter package names
  - Downloads to current directory with all dependencies

## Output Area

All tabs share a common output area at the bottom that displays:
- Command being executed
- Command output (stdout)
- Error messages (stderr)
- Exit status codes

The output uses a monospace font with dark theme for better readability.

## Features

### Modern UI
- **Tabbed Interface**: Organized by feature category
- **Color-Coded Buttons**: Different colors for different action types
  - Green: Run/Execute
  - Purple: Gaming
  - Red: System Health (Doctor)
  - Orange: Security
  - Blue: Flatpak
  - Teal: Quick Actions

- **Responsive Layout**: Adapts to window size
- **Professional Styling**: Modern Qt stylesheet with rounded corners and hover effects

### File Dialogs
- **Export**: Save dialog for exporting package lists
- **Import**: Open dialog for importing package lists

### Confirmation Dialogs
- **Import Packages**: Confirms before installing
- **Flatpak Install**: Confirms before installation
- **Gaming Meta**: Checks for existing installation

### Error Handling
- **Missing Executable**: Warns if fedora-pm is not found
- **Input Validation**: Checks for required inputs
- **Exit Status**: Displays command exit codes

## Running the GUI

### From Source
```bash
python3 fedora-pm-gui.py
```

### From Installed Package
```bash
fedora-pm-gui
```

Or launch from the application menu: **System Tools → Fedora Package Manager**

## Requirements

- Python 3.8+
- PySide6 (Qt for Python)
- fedora-pm CLI installed or in the same directory

### Install Dependencies
```bash
pip install PySide6
```

Or:
```bash
sudo dnf install python3-pyside6
```

## Integration with CLI

The GUI is a thin wrapper around the fedora-pm CLI. It:
- Automatically detects if fedora-pm is installed system-wide
- Falls back to local fedora-pm.py if not installed
- Passes all commands directly to the CLI
- Displays real-time output

## Keyboard Shortcuts

- **Enter** in input fields: Executes the current command
- **Tab**: Navigate between tabs
- **Ctrl+C**: Copy from output area

## Tips

1. **Use Auto-confirm**: Enable the "-y" checkbox for non-interactive operations
2. **Export Regularly**: Use Export Packages to backup your system configuration
3. **Run Doctor Weekly**: Keep your system healthy with regular health checks
4. **Check Security**: Run security audits to stay updated on vulnerabilities
5. **Monitor Output**: Watch the output area for detailed information and errors

## Troubleshooting

### GUI Won't Start
- Check if PySide6 is installed: `python3 -c "import PySide6"`
- Install dependencies: `pip install PySide6`

### Commands Not Working
- Verify fedora-pm is installed: `which fedora-pm`
- Check permissions for sudo operations
- Review output area for error messages

### Flatpak Features Not Working
- Ensure Flatpak is installed: `sudo dnf install flatpak`
- Setup Flathub using the GUI button

## Customization

The GUI stylesheet can be customized by editing the `_apply_styles()` method in `fedora-pm-gui.py`.

### Color Scheme
- Background: `#f5f5f5` (light gray)
- Group boxes: `white`
- Output area: `#2c3e50` (dark blue-gray)
- Accent: `#3498db` (blue)

### Button Colors
- Run: `#27ae60` (green)
- Gaming: `#9b59b6` (purple)
- Doctor: `#e74c3c` (red)
- Security: `#f39c12` (orange)
- Flatpak: `#3498db` (blue)
- Actions: `#16a085` (teal)

## Future Enhancements

Potential additions:
- Progress bars for long operations
- Command history
- Favorites/bookmarks
- Scheduled operations
- System tray integration
- Desktop notifications

## See Also

- [README.md](README.md) - Main documentation
- [FEATURES.md](FEATURES.md) - CLI feature guide
- [QUICK_REFERENCE.md](QUICK_REFERENCE.md) - Command reference
