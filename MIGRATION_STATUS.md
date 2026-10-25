# C++20 / Qt 6 migration status

Target branch: migration/cpp20-qt6

The C++ implementation is being built in parallel with the Rust application. It does not yet replace the Rust application or provide full feature parity.

## Implemented so far
- CMake C++20 project, optional Qt 6 Widgets target, and shell-free process runner with timeouts, captured output, and explicit `sudo --non-interactive` escalation for CLI system mutations.
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
- GitHub Actions is configured to build the CLI, Qt GUI, and tests, run CTest and CLI smoke tests, build/install/uninstall RPMs in a disposable Fedora container, and validate desktop metadata and the CMake installer. Check the latest [Actions runs](https://github.com/ryan1501/fedora-pm/actions) for current results; container CI is not a substitute for real Fedora Workstation/Everything runtime tests.

## Remaining work before replacing Rust
- Audit remaining mutation paths for operation-history logging and continue validating behavior against existing Rust history files.
- Finish Rust CLI compatibility (all original flags, output, and exit codes), including global flags (`--verbose`, `--quiet`, `--sudo`, `--config-dir`), package-specific updates, `deps --tree`, `size --total/--analyze`, `download --dest`, and Flatpak import/export edge cases.
- Implement real self-update support; the current command is only a compatibility placeholder. Keep unsupported legacy workflows explicit rather than silently mapping them to different actions.
- Expand GUI beyond the current core package workflows to Flatpak, repository, kernel, security, driver, backup/import, configuration, and history workflows; test privilege prompts, preview parsing, cancellation, and error handling.
- Strictly validate JSON config compatibility; the current parser handles known fields but is not a general-purpose JSON parser.
- Validate Fedora Workstation and Fedora Everything beyond container CI, including GUI privilege prompts and cancellation. CI builds/installs/uninstalls both RPMs in a disposable Fedora container, but real-machine runtime tests and licensing/documentation review remain outstanding.
- Port the legacy COPR upload scripts to the C++ RPM specs; they remain intentionally disabled so they cannot accidentally publish the old Rust package.

## Safety notes
- Repository addition accepts HTTP(S) URLs only; repository removal remains unimplemented rather than guessing a .repo filename that could delete the wrong configuration.
- System-changing CLI DNF operations use `sudo --non-interactive` when not already root; GUI DNF mutations use `pkexec` and transaction confirmation.
- DNF transaction rollback and kernel removal can alter the system and should be reviewed carefully before confirmation.
- This remains an incomplete development branch and is not ready to merge as the replacement.
