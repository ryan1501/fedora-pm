# C++ migration checklist

The Rust implementation remains authoritative until this checklist is complete and verified on Fedora.

## Foundation and first feature ports
- [x] C++20 / CMake project and optional Qt 6 Widgets target
- [x] Shell-free process runner and initial CLI
- [x] Qt 6 search window with asynchronous DNF
- [x] Basic validation and system information
- [x] DNF search/info/list/install/remove/upgrade/history
- [x] Dependency/reverse-dependency queries and installed groups
- [x] RPM package-list export/import (Flatpak parity still required)
- [x] Flatpak search/list/install/remove/update/info/Flathub setup
- [x] Repository list/info/enable/disable/add/refresh
- [x] Kernel current/list/available/install/remove/remove-old/info
- [x] CachyOS status, kernel list/install, repository enable/disable, CPU feature checks
- [x] Disk space, package sizes, cache size, orphan listing/autoremove
- [x] Package downloads, dependency downloads, offline RPM installation
- [x] RPM changelog and recent changelog entries
- [x] Security update listing, advisory info, CVE query, audit, security upgrade
- [x] Config load/show/get/set for known fields
- [x] Read-only system doctor checks
- [x] GPU detection, NVIDIA operations, CUDA package install, gaming meta install
- [x] DNF history undo by transaction ID/newest transaction
- [x] Unit tests for validation and safe rejection paths
- [x] CI defined for Ubuntu and Fedora; successful run on the latest branch state still required

## Feature parity still required
- [ ] Persistent JSON operation history and logging for successful operations
- [ ] Flatpak-aware export/import parity
- [ ] Package group info/install/remove
- [ ] Changelog update summary/comparison and package cache cleaning
- [ ] Self-update functionality
- [ ] Preserve all existing CLI names, subcommands, flags, output, and exit codes
- [ ] Complete GUI workflows, safe confirmations, progress, cancellation, privilege handling
- [ ] Strict JSON config compatibility and error handling
- [ ] Fedora Workstation and Fedora Everything testing
- [ ] Fedora RPM spec, desktop integration, uninstall behavior, DNF plugin checks
- [ ] Behavior/output comparison against Rust and license/documentation review

## Safety and testing
Do not merge this draft as a replacement yet. Test mutating operations only in disposable Fedora virtual machines until transaction handling, privilege behavior, and failure recovery are reviewed.
