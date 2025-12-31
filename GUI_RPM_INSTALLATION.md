# Fedora Package Manager GUI - RPM Installation Guide

## Overview

The fedora-pm-gui provides a modern Qt-based graphical interface for the fedora-pm command-line tool. This guide covers how to build and install it as a system RPM package.

## Prerequisites

### Build Requirements

To build the RPM, you need the following packages:

```bash
# Install build dependencies
sudo dnf install -y rpm-build python3-devel python3-setuptools desktop-file-utils libappstream-glib

# GUI dependencies (for runtime)
sudo dnf install -y python3 python3-pyside6 python3-pyside6-qtwidgets python3-pyside6-qtgui python3-pyside6-qtcore

# System dependencies
sudo dnf install -y fedora-pm dnf rpm
```

### System Requirements

- Fedora Linux 38+ 
- Python 3.8+
- PySide6 (Qt for Python)
- fedora-pm CLI tool (must be installed first)

## Building the RPM

### Using the Build Script

The easiest way to build the RPM is using the provided build script:

```bash
# Make the build script executable
chmod +x build-gui-rpm.sh

# Run the build script
./build-gui-rpm.sh
```

This will:
1. Create the rpmbuild directory structure
2. Generate a source tarball
3. Build the source and binary RPMs
4. Place the results in `rpmbuild/RPMS/noarch/` and `rpmbuild/SRPMS/`

### Manual Build Process

If you prefer to build manually:

```bash
# Create rpmbuild structure
mkdir -p rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}

# Create source tarball (version 1.0.0)
VERSION=1.0.0
tar -czf rpmbuild/SOURCES/fedora-pm-gui-${VERSION}.tar.gz \
    --exclude-vcs --exclude=rpmbuild --exclude=target \
    --exclude=build --exclude=dist --exclude="*.pyc" \
    --exclude="__pycache__" --exclude=".gitignore" \
    --transform "s|^|fedora-pm-gui-${VERSION}/|" \
    fedora-pm-gui.py fedora-pm-gui-rpm.spec fedora-pm.desktop \
    requirements.txt setup.py fedora_pm_gui/ README.md

# Build the RPM
rpmbuild -ba --define "_topdir $(pwd)/rpmbuild" rpmbuild/SPECS/fedora-pm-gui-rpm.spec
```

## Installing the RPM

### Installing the Built Package

```bash
# Install the binary RPM (adjust path as needed)
sudo dnf install -y rpmbuild/RPMS/noarch/fedora-pm-gui-1.0.0-1.fc*.noarch.rpm
```

### Installing from Source RPM

If you want to rebuild from the source RPM:

```bash
# Install source RPM and rebuild
sudo dnf builddep -y rpmbuild/SRPMS/fedora-pm-gui-1.0.0-1.fc*.src.rpm
rpmbuild --rebuild rpmbuild/SRPMS/fedora-pm-gui-1.0.0-1.fc*.src.rpm
```

## Post-Installation

### Desktop Integration

After installation, the GUI will be available:
- **Application Menu**: System Tools → Fedora Package Manager
- **Command Line**: `fedora-pm-gui`
- **Launcher**: `fedora-pm-gui-launcher` (fallback option)

### Verification

Test the installation:

```bash
# Test the GUI launch (should open Qt interface)
fedora-pm-gui

# Verify desktop file
desktop-file-validate /usr/share/applications/fedora-pm.desktop

# Check appstream metadata
appstream-util validate-relax /usr/share/metainfo/fedora-pm-gui.metainfo.xml
```

## Files Installed

The RPM installs the following files:

```
/usr/bin/fedora-pm-gui                    # Main GUI application
/usr/bin/fedora-pm-gui-launcher          # Alternative launcher
/usr/share/applications/fedora-pm.desktop # Desktop entry
/usr/share/metainfo/fedora-pm-gui.metainfo.xml  # AppStream metadata
/usr/share/doc/fedora-pm-gui/README.md   # Documentation
```

## Dependencies

The package requires:

- **Python Runtime**: `python3`
- **Qt Framework**: `python3-pyside6`, `python3-pyside6-qtwidgets`, `python3-pyside6-qtgui`, `python3-pyside6-qtcore`
- **System Tools**: `fedora-pm >= 1.0.0`, `dnf`, `rpm`

## Troubleshooting

### Build Issues

1. **Permission Denied**: Ensure you have write permissions to the rpmbuild directory
2. **Missing Dependencies**: Install all build dependencies listed in prerequisites
3. **PySide6 Not Found**: Install `python3-pyside6` and related packages

### Runtime Issues

1. **GUI Won't Start**: Check that fedora-pm CLI is installed and in PATH
2. **Qt Errors**: Verify PySide6 installation with `python3 -c "import PySide6"`
3. **Permission Denied**: Some operations require sudo access for system package management

### Desktop Integration

1. **No Menu Entry**: Check desktop file validation and restart desktop session
2. **Icon Missing**: The package uses system icons; custom icons can be added if needed

## Development and Customization

### Modifying the Spec File

Key sections in `fedora-pm-gui-rpm.spec`:
- `%requires`: Add/modify runtime dependencies
- `%install`: Change file installation paths
- `%files`: Add or remove packaged files

### Custom Icons

To add a custom icon:

1. Create `fedora-pm.png` (256x256 pixels)
2. Add to spec file `%install` section:
   ```bash
   install -D -m 644 fedora-pm.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/fedora-pm.png
   ```
3. Add to `%files` section:
   ```
   %{_datadir}/icons/hicolor/256x256/apps/fedora-pm.png
   ```

## Security Considerations

- The GUI requires sudo access for package operations
- Desktop file is validated for security compliance
- AppStream metadata follows Fedora guidelines
- All scripts use system Python interpreter (shebang mangling)

## Support

For issues and support:
- **GitHub**: https://github.com/fedora-pm/fedora-pm/issues
- **Documentation**: Available in `/usr/share/doc/fedora-pm-gui/`
- **CLI Help**: `fedora-pm --help`

## Version History

- **v1.0.0**: Initial RPM release with full Qt GUI functionality