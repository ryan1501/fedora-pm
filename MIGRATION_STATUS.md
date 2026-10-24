# C++20 / Qt 6 migration status

Target branch: migration/cpp20-qt6

The C++ implementation is being built in parallel with the Rust application. It does not yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake C++20 project, optional Qt 6 Widgets target, and shell-free process runner with timeouts and captured output.
- Basic validation, system detection, DNF search/info/list/install/remove/upgrade/history.
- Dependency/reverse-dependency queries and installed package groups.
- DNF package-list export/import with input validation and confirmation.
- Flatpak search/list/install/remove/update/info and idempotent Flathub setup.
- Repository list/info/enable/disable/add/metadata refresh.
- Kernel current/list/available/install/remove/remove-old/info operations; removal protects the running kernel by default.
- CachyOS repository status, kernel listing/install, COPR enable/disable, and x86-64 CPU feature-level checks.
- Disk-space analysis, largest package listing, package-size details, cache size, orphan candidates, and DNF autoremove.
- Package downloads, optional dependency downloads, and offline RPM installation.
- RPM changelog lookup with DNF fallback and recent-entry display.
- Security advisory listing/details, CVE query, audit summary, and security-only upgrade.
- Config load/show/get/set for existing fields, plus read-only system doctor checks.
- GPU detection, NVIDIA status/list/install/remove, CUDA package install, and gaming meta-package install.
- DNF transaction history listing and DNF history undo by transaction ID or newest transaction.
- Unit tests for validation, config persistence, and invalid-input safety paths.
- GitHub Actions workflow includes Ubuntu and Fedora builds of CLI, Qt GUI, CTest, and CLI help.

## Remaining work before replacing Rust
- Implement persistent JSON operation history and connect it to successful package/repository/Flatpak/driver actions.
- Complete Flatpak-aware export/import and group info/install/remove workflows.
- Add changelog update summary/comparison, package cache cleaning, self-update, and remaining CLI flags/options.
- Preserve Rust CLI command names, subcommands, flags, output, and exit codes; add parity tests against Rust behavior.
- Expand GUI beyond search to safe confirmations, progress, cancellation, privilege handling, error reporting, and all workflows.
- Strictly validate JSON config compatibility; the current parser handles known fields but is not a general-purpose JSON parser.
- Validate Fedora Workstation and Fedora Everything, RPM packaging, desktop integration, install/uninstall, DNF plugin requirements, and licensing/documentation.

## Safety notes
- Repository addition accepts HTTP(S) URLs only; repository removal remains unimplemented rather than guessing a .repo filename that could delete the wrong configuration.
- Package mutations may require privileges; automatic elevation is not assumed.
- DNF transaction rollback and kernel removal can alter the system and should be reviewed carefully before confirmation.
- This remains an incomplete development branch and is not ready to merge as the replacement.
