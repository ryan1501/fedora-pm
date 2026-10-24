# C++20 / Qt 6 migration status

Target branch: `migration/cpp20-qt6`

The C++ implementation is being built in parallel with the Rust application. It does **not** yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake-based C++20 project and optional Qt 6 Widgets target.
- Shell-free process runner with captured stdout/stderr and timeout support.
- Initial package-name validation and OS/version/architecture detection.
- DNF search, package info, installed-package listing, install, remove, upgrade, and transaction history.
- Resolved dependency and reverse-dependency queries; installed package-group listing.
- DNF user-installed package list export and validated package-list import with confirmation unless `--yes` is supplied.
- Flatpak search, installed-app listing, install, and removal.
- DNF repository listing, enable/disable operations, and installed/running kernel inventory.
- Disk-space analysis, largest installed package listing, package size details, DNF cache size, orphan candidates, and DNF autoremove.
- DNF package downloads to the current or specified directory, optional dependency downloads, and validated offline RPM installation.
- Installed RPM changelog lookup with DNF fallback and bounded recent-entry display.
- DNF security update listing, advisory lookup, CVE query, security audit summary, and security-only upgrade.
- CLI entry points for the implemented operations.
- Qt 6 Widgets search window; DNF search runs asynchronously so the UI event loop remains responsive.
- Unit tests for validation and invalid-argument paths. GitHub Actions is configured to build the CLI and Qt GUI, run CTest, and smoke-test CLI help; passing status is still to be confirmed.

## Remaining work before replacing Rust
- Port rollback/undo, driver/NVIDIA workflows, gaming setup, system doctor, and configuration management.
- Port persistent JSON operation history and log all successful/failed package operations consistently.
- Add full import/export parity, including the existing Rust Flatpak export format.
- Preserve existing CLI command names, flags, output, and exit codes; add parity tests against Rust behavior.
- Expand GUI coverage beyond package search: safe mutation confirmation, progress, cancellation, privilege handling, error reporting, and the rest of the existing workflows.
- Review repository enable/disable behavior and support for Fedora's installed DNF plugins.
- Test read-only and mutating operations on disposable Fedora Workstation and Fedora Everything installations.
- Validate RPM packaging, desktop integration, install/uninstall behavior, and dependencies.
- Confirm upstream license and update documentation for the final C++ implementation.

## Safety notes
- Import validates every package-list line before invoking DNF and requires confirmation unless `--yes` is given.
- Offline installation accepts existing regular files with the `.rpm` extension only.
- Do not assume package mutations are elevated; privilege behavior must be designed and tested explicitly.
- Process arguments are passed directly, not interpolated into a shell command.
- This remains an incomplete development branch and is not ready to merge as the replacement.
