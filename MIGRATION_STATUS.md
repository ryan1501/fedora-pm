# C++20 / Qt 6 migration status

Target branch: migration/cpp20-qt6

The C++ implementation is being built in parallel with the Rust application. It does not yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake C++20 project, optional Qt 6 Widgets target, and shell-free process runner with timeouts and captured output.
- Basic validation, system detection, DNF search/info/list installed/list available/install/remove/upgrade/history.
- Dependency/reverse-dependency queries, installed groups, and package group info/install/remove.
- DNF package-list export/import with input validation and confirmation; Flatpak export/import has an explicit `--with-flatpak` path and legacy ID parsing, but full parity and edge-case comparison remain open.
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
- GitHub Actions builds the CLI, Qt GUI, and tests, runs CTest and CLI smoke tests, and validates RPM/desktop metadata. The latest verified run passed both Ubuntu and Fedora jobs (run [40135825707](https://github.com/ryan1501/fedora-pm/actions/runs/40135825707)). This is CI evidence only; it is not a substitute for real Fedora Workstation/Everything runtime tests.

## Remaining work before replacing Rust
- Audit remaining mutation paths for operation-history logging and continue validating behavior against existing Rust history files.
- Finish Rust CLI compatibility (all original flags, output, and exit codes) and compare Flatpak export/import edge cases against the Rust version.
- Implement real self-update support; the current command is only a compatibility placeholder. Align remaining CLI names, flags, output, and exit codes with the Rust CLI (for example, Rust's `whats-new` spelling differs from the C++ `whatsnew` command).
- Expand GUI beyond the current core package workflows to Flatpak, repository, kernel, security, driver, backup/import, configuration, and history workflows; test privilege prompts, preview parsing, cancellation, and error handling.
- Strictly validate JSON config compatibility; the current parser handles known fields but is not a general-purpose JSON parser.
- Validate Fedora Workstation and Fedora Everything, RPM packaging, desktop integration, install/uninstall, DNF plugin requirements, and licensing/documentation. Latest GUI transaction-preview and Flatpak mutation changes passed Ubuntu/Fedora CI; no Fedora Workstation/Everything runtime test is claimed yet.

## Safety notes
- Repository addition accepts HTTP(S) URLs only; repository removal remains unimplemented rather than guessing a .repo filename that could delete the wrong configuration.
- Package mutations may require privileges; automatic elevation is not assumed.
- DNF transaction rollback and kernel removal can alter the system and should be reviewed carefully before confirmation.
- This remains an incomplete development branch and is not ready to merge as the replacement.
