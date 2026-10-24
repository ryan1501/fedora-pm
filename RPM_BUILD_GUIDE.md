# RPM packaging guide for the C++ migration

The C++ migration uses C++20, CMake, and Qt 6 Widgets. This guide replaces the old Python/PySide6 RPM instructions, which refer to files and a build script that are no longer the supported path.

**Packaging status:** initial CLI and GUI spec files and desktop metadata exist, and CI parses the GUI spec and validates the desktop entry. A complete RPM build/install/uninstall cycle has not yet been validated on a real Fedora Workstation or Fedora Everything installation. Treat the packaging as experimental.

## Build and test from source

On Fedora Workstation or Fedora Everything, install the development dependencies:

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel
```

From the repository root:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=ON -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

To build only the CLI on a minimal installation without Qt 6 Widgets:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=OFF -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Packaging assets

- `fedora-pm-rpm.spec` — CLI package spec
- `fedora-pm-gui-rpm.spec` — Qt 6 GUI package spec
- `fedora-pm.desktop` — desktop launcher entry
- `fedora-pm.metainfo.xml` — AppStream metadata

The specs currently expect a versioned source archive from the repository's release/tag URL. Do not assume that running `rpmbuild -ba` directly against the repository root works, and do not use the old `build-rpm.sh` instructions.

## Required validation before distribution

Use a disposable Fedora virtual machine and verify:

1. Build both spec files against the same versioned source archive and inspect build logs.
2. Confirm the CLI and GUI packages have correct runtime dependencies, including Qt 6 Widgets for the GUI and required DNF/plugin/Flatpak tooling for the commands that use them.
3. Install both packages and check `fedora-pm --help`, `fedora-pm --version`, and `fedora-pm-gui`.
4. Verify `/usr/share/applications/fedora-pm.desktop` with `desktop-file-validate`, and check the AppStream metadata installation path.
5. Exercise GUI privilege prompts, transaction preview/confirmation, cancellation, and error paths.
6. Remove the packages and confirm package-owned files are removed without deleting user configuration or history.
7. Repeat on Fedora Workstation and on a minimal Fedora Everything installation with a graphical session added.

Until those checks are complete, use the CMake workflow above for development and testing rather than treating the RPMs as ready for general distribution. See [`docs/CPP_MIGRATION_PLAN.md`](docs/CPP_MIGRATION_PLAN.md) and [`BUILD_RPM_QUICKSTART.md`](BUILD_RPM_QUICKSTART.md) for migration status and build commands.
