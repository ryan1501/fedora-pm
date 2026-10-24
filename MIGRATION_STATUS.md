# C++20 / Qt 6 migration status

Target branch: migration/cpp20-qt6

The C++ implementation is being built in parallel with the Rust application. It does not yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake C++20 project, optional Qt 6 Widgets target, and shell-free process runner with timeouts and captured output.
- Basic validation, system detection, DNF search/info/list installed/list available/install/remove/upgrade/history.
- Dependency/reverse-dependency queries, installed groups, and package group info/install/remove.
- DNF + Flatpak package-list export/import with input validation, legacy export compatibility, and confirmation.
- Flatpak search/list/install/remove/update/info and idempotent Flathub setup.
- Repository list/info/enable/disable/add/metadata refresh.
- Kernel current/list/available/install/remove/remove-old/info operations; removal protects the running kernel by default.
- CachyOS repository status, kernel listing/install, COPR enable/disable, and x86-64 CPU feature-level checks.
- DNF cache cleanup; disk-space analysis, largest package listing, package-size details, cache size, orphan candidates, and DNF autoremove.
- Package downloads, optional dependency downloads, and offline RPM installation.
- RPM changelog lookup with DNF fallback, recent-entry display, update summary with changelog previews, and explicit changelog-comparison limitations.
- Security advisory listing/details, CVE query, audit summary, and security-only upgrade.
- Config load/show/get/set for existing fields, plus read-only system doctor checks.
- GPU detection, NVIDIA status/list/install/remove, CUDA package install, and gaming meta-package install.
- DNF transaction history listing and DNF history undo by transaction ID or newest transaction.
- Persistent JSON operation history with timestamps, recent-history display, and CLI logging for many successful mutation commands.
- Qt 6 Widgets GUI for package search/info/listing, install/remove/update, captured output, busy state, cancellation, and DNF transaction preview before user approval.
- Unit tests for validation, config persistence, history JSON round-trip, and invalid-input safety paths.
- GitHub Actions workflow includes Ubuntu and Fedora builds of CLI, Qt GUI, CTest, and CLI help.

## Remaining work before replacing Rust
- Finish connecting operation history to every mutation path and validate compatibility with existing Rust history files.
- Complete Flatpak-aware export/import and full Rust CLI compatibility (including all original flags and output behavior).
- Add self-update support and align remaining CLI names, flags, output, and exit codes with the Rust CLI (for example, Rust's `whats-new` spelling differs from the C++ `whatsnew` command).
- Expand GUI beyond the current core package workflows to Flatpak, repository, kernel, security, driver, backup/import, configuration, and history workflows; test privilege prompts, preview parsing, cancellation, and error handling.
- Strictly validate JSON config compatibility; the current parser handles known fields but is not a general-purpose JSON parser.
- Validate Fedora Workstation and Fedora Everything, RPM packaging, desktop integration, install/uninstall, DNF plugin requirements, and licensing/documentation. Latest GUI transaction-preview changes are awaiting CI; no Fedora Workstation/Everything runtime test is claimed yet.

## Safety notes
- Repository addition accepts HTTP(S) URLs only; repository removal remains unimplemented rather than guessing a .repo filename that could delete the wrong configuration.
- Package mutations may require privileges; automatic elevation is not assumed.
- DNF transaction rollback and kernel removal can alter the system and should be reviewed carefully before confirmation.
- This remains an incomplete development branch and is not ready to merge as the replacement.
