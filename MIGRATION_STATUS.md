# C++20 / Qt 6 migration status

Target branch: migration/cpp20-qt6

The C++ implementation is being built in parallel with the Rust application. It does not yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake-based C++20 project, optional Qt 6 Widgets target, and shell-free process runner with timeout/captured output.
- Initial validation, OS detection, and DNF search/info/list/install/remove/upgrade/history.
- Resolved dependencies, reverse dependencies, and installed package-group listing.
- DNF user-installed package-list export/import with line validation and confirmation.
- Flatpak search/list/install/remove/update/info and idempotent Flathub remote setup.
- Repository listing, info, enable/disable, URL-based add, and metadata refresh.
- Running/installed kernel inventory.
- Disk-space analysis, largest package listing, package-size details, cache size, orphan candidates, and DNF autoremove.
- Package downloads, optional dependency downloads, and offline RPM installation.
- RPM changelog lookup with DNF fallback and recent-entry display.
- Security advisory listing/details, CVE query, audit summary, and security-only upgrade.
- JSON-compatible app config load/show/get/set for the existing configuration keys.
- Read-only system doctor checks.
- GPU detection, NVIDIA status/list/install/remove, CUDA install, and gaming meta-package install.
- DNF transaction history listing and DNF history undo by transaction ID or newest transaction.
- Unit tests for input validation, configuration persistence, and safe rejection paths.
- GitHub Actions workflow to build CLI/GUI, run CTest, and smoke-test CLI help. A successful run against the latest complete branch state is still required.

## Remaining work before replacing Rust
- Implement persistent JSON operation history and connect it to all successful package/repository/Flatpak/driver actions.
- Complete kernel install/remove/cleanup workflows and CachyOS kernel/repository/CPU checks.
- Complete Flatpak-aware export/import and group info/install/remove workflows.
- Add changelog update summary/comparison, package cache cleaning, self-update, and remaining CLI flags/options.
- Preserve Rust CLI command names, subcommands, flags, output, and exit codes; add parity tests against Rust behavior.
- Expand GUI beyond search to safe confirmations, progress, cancellation, privilege handling, error reporting, and all workflows.
- Review config JSON parsing for strict compatibility and test on Fedora Workstation and Fedora Everything.
- Validate RPM packaging, desktop integration, install/uninstall behavior, DNF plugin requirements, and licensing/documentation.

## Safety notes
- The config implementation handles the known configuration fields but still needs strict JSON/parser compatibility testing.
- Repository addition only accepts HTTP(S) URLs; repository removal is not implemented because guessing a .repo filename can delete the wrong configuration.
- Package mutations may require privileges; do not assume automatic elevation.
- This remains an incomplete development branch and is not ready to merge as the replacement.
