# C++ migration checklist

The Rust implementation remains authoritative until this checklist is complete and verified on Fedora.

## Foundation
- [x] C++20 / CMake project setup
- [x] Initial CLI
- [x] Initial Qt 6 Widgets search window
- [x] Shell-free process invocation abstraction
- [x] Basic package validation and system information
- [x] DNF search/info/list/install/remove/upgrade operations (initial implementation)
- [x] DNF transaction history listing (initial implementation)
- [x] DNF dependency and reverse-dependency queries (initial implementation)
- [x] Installed DNF package-group listing (initial implementation)
- [x] DNF user-installed package export and validated package-list import
- [x] Flatpak search/list/install/remove operations (initial implementation)
- [x] Repository list/enable/disable (initial implementation)
- [x] Kernel inventory (initial implementation)
- [x] Asynchronous DNF search in Qt GUI
- [x] CI workflow for C++ build, Qt GUI build, tests, and CLI help smoke test

## Feature parity still required
- [ ] Preserve existing CLI command names, flags, output, and exit codes
- [ ] Rollback/undo with safe transaction validation and recovery
- [ ] Driver detection and NVIDIA workflows
- [ ] Gaming setup and related packages
- [ ] Security checks and system doctor/diagnostics
- [ ] Configuration management
- [ ] Flatpak-aware export/import parity with the Rust implementation
- [ ] Changelog and disk-space tools
- [ ] Download support
- [ ] Complete GUI workflows, safe confirmations, progress reporting, cancellation, and privilege handling
- [ ] Logging and structured error reporting parity
- [ ] Audit repository enable/disable behavior and DNF plugin availability
- [ ] Fedora RPM spec, desktop integration, uninstall behavior
- [ ] Automated tests on Fedora Workstation and Fedora Everything
- [ ] Compare outputs and behavior against the Rust implementation
- [ ] Review license and project documentation before final cutover

## Safety and testing
Do not merge this draft as a replacement yet. Test mutating operations only in disposable Fedora virtual machines until transaction handling, privilege behavior, and failure recovery are reviewed.
