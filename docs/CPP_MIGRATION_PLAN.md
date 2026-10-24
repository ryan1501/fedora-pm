# C++ migration checklist

The Rust implementation remains authoritative until this checklist is complete and verified on Fedora.

## Foundation and first feature ports
- [x] C++20 / CMake project and optional Qt 6 Widgets target
- [x] Shell-free process runner and initial CLI
- [x] Qt 6 search window with asynchronous DNF
- [x] Basic validation and system information
- [x] DNF search/info/list installed/list available/install/remove/upgrade/history
- [x] Dependency/reverse-dependency queries and installed groups
- [x] Package group info/install/remove
- [x] RPM package-list export/import (Flatpak parity still required)
- [x] Flatpak search/list/install/remove/update/info/Flathub setup
- [x] Repository list/info/enable/disable/add/refresh
- [x] Kernel current/list/available/install/remove/remove-old/info
- [x] CachyOS status, kernel list/install, repository enable/disable, CPU feature checks
- [x] Disk space, package sizes, cache cleanup, orphan listing/autoremove
- [x] Package downloads, dependency downloads, offline RPM installation
- [x] RPM changelog and recent changelog entries
- [x] Security update listing, advisory info, CVE query, audit, security upgrade
- [x] Config load/show/get/set for known fields
- [x] Read-only system doctor checks
- [x] GPU detection, NVIDIA operations, CUDA package install, gaming meta install
- [x] DNF history undo by transaction ID/newest transaction
- [x] Persistent JSON operation history and logging for common CLI mutation paths and confirmed GUI DNF transactions
- [x] Unit tests for validation, config persistence, history round-trip, and safe rejection paths
- [x] CI build, unit tests, and CLI smoke tests pass on Ubuntu and Fedora for the current GUI/Flatpak-history implementation

## Feature parity still required
- [x] Log successful CLI mutations and confirmed GUI DNF/Flatpak transactions; test reading legacy Rust-format history JSON, including Unicode escapes and RFC3339 timestamps
- [x] Flatpak-aware export/import, including legacy Rust comment-style Flatpak IDs and strict application-ID validation
- [x] Changelog update summary/comparison commands (`whats-new` and `changelog-compare`) ported; arbitrary-version filtering remains intentionally unavailable, as in the Rust implementation
- [ ] Self-update functionality
- [ ] Preserve all existing CLI names, subcommands, flags, output, and exit codes
- [ ] Complete GUI workflows, safe confirmations, progress, cancellation, privilege handling
- [ ] Strict JSON config compatibility and error handling
- [ ] Fedora Workstation and Fedora Everything testing
- [x] Initial CLI/GUI RPM spec files and desktop/metainfo integration assets
- [ ] Validate RPM builds/install/uninstall on Fedora; verify DNF plugin dependencies and desktop integration
- [ ] Behavior/output comparison against Rust and license/documentation review

## Safety and testing
Do not merge this draft as a replacement yet. Test mutating operations only in disposable Fedora virtual machines until transaction handling, privilege behavior, and failure recovery are reviewed.
