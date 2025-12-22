# Building Fedora Package Manager RPM

## Quick Start

1. Install build dependencies:
```bash
sudo dnf install rpm-build python3-devel
```

2. Build the RPM:
```bash
./build-rpm.sh
```

3. Install the built RPM:
```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

## What Gets Built

The build process creates:

- **Binary RPM**: `rpmbuild/RPMS/noarch/fedora-pm-1.0.0-1.noarch.rpm`
  - Contains the CLI tool (`fedora-pm`)
  - Contains the GUI tool (`fedora-pm-gui`)
  - Contains the Python module (`fedora_pm.py`)
  - Contains the desktop entry file

- **Source RPM**: `rpmbuild/SRPMS/fedora-pm-1.0.0-1.src.rpm`
  - Contains the source tarball and spec file
  - Can be used to rebuild on other systems

## Installation Locations

After installation, files are placed in:

- `/usr/bin/fedora-pm` - CLI executable
- `/usr/bin/fedora-pm-gui` - GUI executable
- `/usr/share/fedora-pm/fedora_pm.py` - Python module
- `/usr/share/applications/fedora-pm.desktop` - Desktop entry
- `/usr/share/doc/fedora-pm/` - Documentation

## Testing the Build

After building, you can test the RPM without installing:

```bash
# Check the RPM contents
rpm -qlp rpmbuild/RPMS/noarch/fedora-pm-*.rpm

# Check for issues
rpmlint rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

## Manual Build Steps

If you prefer to build manually instead of using the script:

1. Create source tarball:
```bash
tar -czf ~/rpmbuild/SOURCES/fedora-pm-1.0.0.tar.gz \
    --exclude='rpmbuild' \
    --exclude='.git' \
    fedora-pm.py fedora-pm-gui.py fedora_pm.py \
    fedora-pm.desktop README.md requirements.txt install.sh
```

2. Copy spec file:
```bash
cp fedora-pm.spec ~/rpmbuild/SPECS/
```

3. Build:
```bash
rpmbuild -ba ~/rpmbuild/SPECS/fedora-pm.spec
```

## Troubleshooting

### Build fails with "No such file or directory"

Make sure you have the `rpmbuild` directory structure:
```bash
mkdir -p ~/rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
```

Or use the build script which creates the structure automatically.

### Import errors in GUI

After installation, the GUI should find the module automatically. If not:
- Check that `fedora_pm.py` is in `/usr/share/fedora-pm/`
- Verify Python can import it: `python3 -c "import fedora_pm"`

### Missing dependencies

The spec file requires:
- `python3 >= 3.6`
- `python3-tkinter` (for GUI)
- `dnf`, `rpm`, `sudo` (system tools)

Install missing dependencies with:
```bash
sudo dnf install python3-tkinter
```

