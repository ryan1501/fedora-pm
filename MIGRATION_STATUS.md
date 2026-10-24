# C++20 / Qt 6 migration status

Target branch: `migration/cpp20-qt6`

The C++ implementation is being built in parallel with the Rust application. It does **not** yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake-based C++20 project and optional Qt 6 Widgets target.
- Shell-free process runner with captured stdout/stderr and timeout support.
- Initial package-name validation and OS/version/architecture detection.
- DNF search, package info, installed-package listing, install, remove, upgrade, and transaction history.
- Flatpak search, installed-app listing, install, and removal.
- DNF repository listing, enable/disable operations, and installed/running kernel inventory.
- CLI entry points for the implemented operations.
- Qt 6 Widgets search window; DNF search now runs asynchronously so the UI event loop remains responsive.
- Initial validation tests and a CI workflow that configures/builds the CLI, Qt GUI, and tests, then runs CTest and a CLI help smoke test.

## Remaining work before replacing Rust
- Inventory and port every existing Rust command/module, including rollback, drivers/NVIDIA, gaming, security, doctor, configuration, export/import, groups, changelog, dependencies, disk-space tools, downloads, and advanced validation.
- Preserve existing CLI command names, flags, output, and exit codes; add parity tests against Rust behavior.
- Expand GUI coverage beyond package search: safe mutation confirmation, progress, cancellation, privilege handling, error reporting, and the rest of the existing workflows.
- Review repository enable/disable behavior and support for Fedora's installed DNF plugins.
- Test read-only and mutating operations on disposable Fedora Workstation and Fedora Everything installations.
- Validate RPM packaging, desktop integration, install/uninstall behavior, and dependencies.
- Confirm upstream license and update documentation for the final C++ implementation.

## Safety notes
- Do not assume package mutations are elevated; privilege behavior must be designed and tested explicitly.
- Process arguments are passed directly, not interpolated into a shell command.
- This remains an incomplete development branch and is not ready to merge as the replacement application.
