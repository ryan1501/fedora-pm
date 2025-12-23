# Building Fedora Package Manager as an RPM GUI Application

This guide explains how to build and package the Fedora Package Manager (CLI + GUI) as an RPM package that can be installed system-wide and appears in your applications menu.

## Quick Start

### 1. Install Build Dependencies

```bash
sudo dnf install rpm-build rpmdevtools python3-pyside6 python3-devel
```

### 2. Build the RPM

```bash
./build-rpm.sh
```

### 3. Install the RPM

```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

After installation:
- **CLI**: Run `fedora-pm --help` from terminal
- **GUI**: Find "Fedora Package Manager" in your applications menu or run `fedora-pm-gui`

## What Gets Installed

When you install the RPM, the following files are installed:

### Executables
- `/usr/bin/fedora-pm` - CLI interface
- `/usr/bin/fedora-pm-gui` - GUI interface

### Desktop Integration
- `/usr/share/applications/fedora-pm.desktop` - Desktop entry file (makes it appear in applications menu)

### Documentation
- `/usr/share/doc/fedora-pm/README.md` - Documentation

## Manual Build Process

If you prefer to build manually:

### Step 1: Create Source Tarball

```bash
# Create temporary directory structure
TEMP_DIR=$(mktemp -d)
mkdir -p "$TEMP_DIR/fedora-pm-1.0.0"

# Copy source files
cp fedora-pm.py "$TEMP_DIR/fedora-pm-1.0.0/"
cp fedora-pm-gui.py "$TEMP_DIR/fedora-pm-1.0.0/"
cp fedora-pm.desktop "$TEMP_DIR/fedora-pm-1.0.0/"
cp README.md "$TEMP_DIR/fedora-pm-1.0.0/"
cp requirements.txt "$TEMP_DIR/fedora-pm-1.0.0/"

# Create tarball
cd "$TEMP_DIR"
tar -czf ~/rpmbuild/SOURCES/fedora-pm-1.0.0.tar.gz fedora-pm-1.0.0
cd -
rm -rf "$TEMP_DIR"
```

### Step 2: Copy Spec File

```bash
cp rpmbuild/SPECS/fedora-pm.spec ~/rpmbuild/SPECS/
```

### Step 3: Build RPM

```bash
cd ~/rpmbuild/SPECS
rpmbuild -ba fedora-pm.spec
```

### Step 4: Install

```bash
sudo dnf install ~/rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

## Spec File Details

The `fedora-pm.spec` file defines:

### Dependencies
- `python3 >= 3.6` - Python runtime
- `python3-pyside6` - Qt GUI framework
- `dnf` - Package manager
- `rpm` - RPM tools
- `sudo` - For privilege escalation

### Installation Locations
- **Binaries**: `/usr/bin/` (system PATH)
- **Desktop Entry**: `/usr/share/applications/` (applications menu)
- **Documentation**: `/usr/share/doc/fedora-pm/`

## Desktop Entry File

The `fedora-pm.desktop` file makes the GUI appear in your applications menu:

```ini
[Desktop Entry]
Name=Fedora Package Manager
Exec=fedora-pm-gui
Icon=system-software-install
Categories=System;PackageManager;Settings;
```

This file is automatically installed to `/usr/share/applications/` by the RPM.

## Verifying Installation

After installation, verify everything works:

### Check CLI
```bash
fedora-pm --help
```

### Check GUI
```bash
fedora-pm-gui
```

### Check Desktop Entry
```bash
# Should show the desktop entry
ls /usr/share/applications/fedora-pm.desktop

# Should appear in applications menu
gtk4-launcher fedora-pm.desktop  # or use your desktop environment's app launcher
```

## Troubleshooting

### GUI Doesn't Appear in Applications Menu

1. **Refresh desktop database:**
   ```bash
   sudo update-desktop-database
   ```

2. **Check desktop entry file:**
   ```bash
   desktop-file-validate /usr/share/applications/fedora-pm.desktop
   ```

3. **Verify file permissions:**
   ```bash
   ls -l /usr/share/applications/fedora-pm.desktop
   # Should be readable by all
   ```

### PySide6 Not Found

If the GUI fails to launch:
```bash
# Install PySide6
sudo dnf install python3-pyside6

# Verify installation
python3 -c "import PySide6; print(PySide6.__version__)"
```

### Build Errors

**"No source number 0":**
- Ensure the source tarball exists in `rpmbuild/SOURCES/`
- Check that `Source0` in spec file matches tarball name

**"Bad exit status from %prep":**
- Ensure tarball has correct top-level directory structure
- The tarball should extract to `fedora-pm-1.0.0/` directory

**"File not found":**
- Verify all source files exist before building
- Check file paths in spec file match actual files

## Customization

### Changing Installation Paths

Edit `fedora-pm.spec` and modify the `%install` section:

```spec
%install
# Custom installation directory
mkdir -p %{buildroot}/opt/fedora-pm
install -m 755 fedora-pm-gui.py %{buildroot}/opt/fedora-pm/
```

### Adding an Icon

1. Create or obtain an icon file (e.g., `fedora-pm.png` or `fedora-pm.svg`)
2. Add to spec file:
   ```spec
   # In %install section
   install -m 644 fedora-pm.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/
   ```
3. Update desktop file:
   ```ini
   Icon=fedora-pm
   ```

### Adding Additional Files

To include additional files (configs, scripts, etc.):

1. Copy files in `%install` section
2. Add to `%files` section:
   ```spec
   %files
   %{_bindir}/fedora-pm-gui
   %config(noreplace) %{_sysconfdir}/fedora-pm/config.conf
   ```

## Distribution

### Creating a Source RPM (SRPM)

The build script automatically creates an SRPM:
```bash
# SRPM location
rpmbuild/SRPMS/fedora-pm-1.0.0-1.src.rpm
```

### Sharing the RPM

You can share the built RPM file:
```bash
# Binary RPM
rpmbuild/RPMS/noarch/fedora-pm-1.0.0-1.noarch.rpm

# Source RPM (for rebuilding)
rpmbuild/SRPMS/fedora-pm-1.0.0-1.src.rpm
```

### Installing from RPM File

```bash
sudo dnf install /path/to/fedora-pm-1.0.0-1.noarch.rpm
```

## Advanced: Creating a Repository

To create your own RPM repository:

```bash
# Install createrepo
sudo dnf install createrepo

# Create repository directory
mkdir -p ~/rpm-repo

# Copy RPMs
cp rpmbuild/RPMS/noarch/*.rpm ~/rpm-repo/

# Create repository metadata
createrepo ~/rpm-repo

# Add repository to system
sudo tee /etc/yum.repos.d/my-repo.repo <<EOF
[my-repo]
name=My Custom Repository
baseurl=file://$HOME/rpm-repo
enabled=1
gpgcheck=0
EOF
```

## Summary

The RPM package provides:
- ✅ System-wide installation
- ✅ Desktop menu integration
- ✅ Proper dependency management
- ✅ Easy uninstallation (`sudo dnf remove fedora-pm`)
- ✅ Professional packaging standards

Use `./build-rpm.sh` for the easiest build process!

