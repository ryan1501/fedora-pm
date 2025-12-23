# Quick Start: Build RPM GUI App

## One-Command Build

```bash
./build-rpm.sh
```

Then install:
```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-pm-*.rpm
```

## What This Does

1. ✅ Creates source tarball with all necessary files
2. ✅ Builds RPM package using the spec file
3. ✅ Installs CLI (`fedora-pm`) and GUI (`fedora-pm-gui`) to `/usr/bin/`
4. ✅ Adds desktop entry so GUI appears in applications menu
5. ✅ Handles all dependencies (PySide6, dnf, etc.)

## After Installation

- **CLI**: `fedora-pm --help`
- **GUI**: `fedora-pm-gui` or find "Fedora Package Manager" in applications menu

## Prerequisites

```bash
sudo dnf install rpm-build rpmdevtools python3-pyside6 python3-devel
```

## Files Created

- `rpmbuild/RPMS/noarch/fedora-pm-*.rpm` - Binary RPM (install this)
- `rpmbuild/SRPMS/fedora-pm-*.rpm` - Source RPM (for rebuilding)

For detailed information, see `RPM_BUILD_GUIDE.md`.

