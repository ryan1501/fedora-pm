# Fedora Package Manager - GUI Version

This document explains how to build and install the GUI version of Fedora Package Manager as an RPM package.

## Building the RPM

### Prerequisites

Make sure you have the required build tools installed:

```bash
sudo dnf install rpm-build rpmdevtools
```

### Build Steps

1. Run the build script:

```bash
./build-rpm.sh
```

This will:
- Create a source tarball
- Build both binary and source RPMs
- Place them in the `rpmbuild/RPMS` and `rpmbuild/SRPMS` directories

2. Install the RPM:

```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

## Manual Build

If you prefer to build manually:

1. Create source tarball:
```bash
tar -czf ~/rpmbuild/SOURCES/fedora-pm-1.0.0.tar.gz \
    fedora-pm.py fedora-pm-gui.py fedora_pm.py \
    fedora-pm.desktop README.md requirements.txt install.sh
```

2. Copy spec file:
```bash
cp fedora-pm.spec ~/rpmbuild/SPECS/
```

3. Build RPM:
```bash
rpmbuild -ba ~/rpmbuild/SPECS/fedora-pm.spec
```

## Using the GUI

After installation, you can launch the GUI in several ways:

1. From the command line:
```bash
fedora-pm-gui
```

2. From the application menu:
   - Look for "Fedora Package Manager" in your applications menu
   - It should be under System or Package Manager category

3. Using the CLI (still available):
```bash
fedora-pm install vim git
fedora-pm search python
```

## GUI Features

The GUI provides a tabbed interface with:

- **Packages Tab**: Install, remove, update, list, and get info about packages
- **Search Tab**: Search for packages with results displayed in a table
- **Kernels Tab**: Manage kernels (list, install, remove old kernels, CachyOS kernels)
- **Drivers Tab**: Check driver status, detect GPUs, install/remove Nvidia drivers
- **System Tab**: Clean cache and view operation history

All operations run in background threads to keep the GUI responsive, and output is displayed in the output area at the bottom of the window.

## Requirements

- Python 3.6+
- tkinter (usually included with Python)
- dnf and rpm (Fedora package managers)
- sudo access for package operations

## Troubleshooting

### GUI doesn't start

- Check that tkinter is installed: `python3 -m tkinter`
- If missing, install it: `sudo dnf install python3-tkinter`

### Import errors

- Make sure `fedora_pm.py` is in the same directory as `fedora-pm-gui.py` or in the system path
- After RPM installation, the module should be in `/usr/share/fedora-pm/`

### Permission errors

- The GUI will prompt for sudo password when needed
- Make sure your user has sudo privileges

