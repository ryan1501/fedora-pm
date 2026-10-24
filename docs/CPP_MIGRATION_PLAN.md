# C++ migration checklist

The Rust implementation remains authoritative until this checklist is complete and verified on Fedora.

## Foundation and first feature ports
- [x] C++20 / CMake project setup
- [x] Initial CLI
- [x] Initial Qt 6 Widgets search window
- [x] Shell-free process invocation abstraction
- [x] Basic package validation and system information
- [x] DNF search/info/list/install/remove/upgrade operations (initial implementation)
- [x] DNF transaction history listing
- [x] DNF dependency and reverse-dependency queries
- [x] Installed DNF package-group listing
- [x] DNF user-installed package export and validated package-list import
- [x] Flatpak search/list/install/remove operations (initial implementation)
- [x] Repository list/enable/disable (initial implementation)
- [x] Kernel inventory
- [x] Disk space analysis, largest packages, package-size inspection, cache size, orphan listing and autoremove
- [x] Package downloads, optional dependency downloads, offline RPM install
- [x] RPM changelog and recent changelog entries
- [x] Security advisory listing, advisory info, CVE query, audit summary and security-only update
- [x] Asynchronous DNF search in Qt GUI
- [x] Unit tests for validation and invalid arguments
- [x] CI workflow for C++ build, Qt GUI build, tests, and CLI help smoke test (awaiting confirmed pass)

## Feature parity still required
- [ ] Preserve existing CLI command names, flags, output, and exit codes
- [ ] Rollback/undo with safe transaction validation and recovery
- [ ] Driver detection and NVIDIA workflows
- [ ] Gaming setup and related packages
- [ ] System doctor/diagnostics
- [ ] Configuration management
- [ ] Persistent JSON operation history and consistent action logging
- [ ] Flatpak-aware export/import parity with the Rust implementation
- [ ] Changelog update summary / compare workflows
- [ ] Complete GUI workflows, safe confirmations, progress reporting, cancellation, and privilege handling
- [ ] Logging and structured error reporting parity
- [ ] Audit repository enable/disable behavior and DNF plugin availability
- [ ] Fedora RPM spec, desktop integration, uninstall behavior
- [ ] Automated tests on Fedora Workstation and Fedora Everything
- [ ] Compare outputs and behavior against the Rust implementation
- [ ] Review license and project documentation before final cutover

## Safety and testing
Do not merge this draft as a replacement yet. Test mutating operations only in disposable Fedora virtual machines until transaction handling, privilege behavior, and failure recovery are reviewed.
