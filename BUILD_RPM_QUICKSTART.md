# Build and test the C++ migration

This guide replaces the old PySide6/Rust-era RPM instructions. The migration now uses C++20, CMake, and Qt 6 Widgets. The RPM spec files and desktop metadata are present, but building and installing RPMs on Fedora is still a validation task; do not assume the old one-command `build-rpm.sh` workflow exists.

## Build on Fedora Workstation or Fedora Everything

Install the development dependencies:

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel
```

From the repository root:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=ON -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

If you are on a minimal system without Qt 6 Widgets available, build and test the CLI only:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=OFF -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Run the built applications

```bash
./build/fedora-pm --help
./build/fedora-pm --version
./build/fedora-pm-gui
```

The GUI command is available only when CMake finds Qt 6 Widgets and GUI building is enabled.

## RPM packaging status

The repository contains `fedora-pm-rpm.spec` for the CLI, `fedora-pm-gui-rpm.spec` for the GUI, plus `fedora-pm.desktop` and `fedora-pm.metainfo.xml`. CI builds both RPMs and validates installation/removal in a disposable Fedora container, including desktop integration and file cleanup. Real Fedora Workstation and Fedora Everything testing is still outstanding. The specs expect a versioned source archive; CI creates one from the checkout while excluding checked-in legacy build artifacts. Complete real-machine validation and licensing review before distribution.

See [`docs/CPP_MIGRATION_PLAN.md`](docs/CPP_MIGRATION_PLAN.md) for remaining parity and packaging checks, and [`RPM_BUILD_GUIDE.md`](RPM_BUILD_GUIDE.md) for additional packaging notes.
