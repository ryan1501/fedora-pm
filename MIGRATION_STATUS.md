# C++20 / Qt 6 migration status

Target branch: `migration/cpp20-qt6`

This branch starts the migration in parallel with the existing Rust implementation. It does **not** yet replace the Rust application or provide feature parity.

## Added in the initial stage
- CMake-based C++20 build.
- Shell-free process runner with captured stdout/stderr and a timeout.
- Initial package-name validation.
- Basic OS/version/architecture detection.
- Initial DNF operations: search, info, list installed, install, remove, upgrade.
- A small CLI and an initial Qt 6 Widgets package-search window.
- Basic validation tests.

## Known gaps before this can replace the Rust application
- The existing Rust feature set must be inventoried and ported module-by-module, including Flatpak, drivers/NVIDIA, gaming, history, rollback, repositories, kernel management, security, doctor, config, export/import, groups, changelog, dependency and disk-space tools, download, and validation.
- Existing CLI syntax and output compatibility need parity tests.
- The Qt window currently runs DNF synchronously and needs asynchronous execution, progress, cancellation, and privilege handling before production use.
- DNF write operations must be exercised on Fedora Workstation and Fedora Everything installations in disposable VMs.
- RPM packaging and installation/uninstallation tests are not yet validated.
- Confirm the upstream license before copying or changing licensing metadata.

## Safety notes
- Package mutations are not automatically elevated by this application. Run from a suitable privileged environment when needed.
- The process runner passes arguments directly rather than interpolating a shell command.
- This is an early development scaffold, not a production-ready package manager.
