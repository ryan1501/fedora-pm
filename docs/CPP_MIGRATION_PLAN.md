# C++ migration checklist

The Rust implementation remains authoritative until this checklist is complete and verified on Fedora.

## Foundation
- [x] C++20 / CMake project setup
- [x] Initial CLI
- [x] Initial Qt 6 Widgets search window
- [x] Shell-free process invocation abstraction
- [x] Basic package validation and system information
- [x] DNF package operations (initial implementation)
- [x] Flatpak operations (initial implementation)
- [x] Repository list/enable/disable (initial implementation)
- [x] Kernel inventory (initial implementation)

## Feature parity still required
- [ ] Preserve the existing CLI command names, flags, output, and exit codes
- [ ] Package history and transaction details
- [ ] Rollback/undo with safe transaction validation
- [ ] Driver detection and NVIDIA workflows
- [ ] Gaming setup and related packages
- [ ] Security checks and system doctor/diagnostics
- [ ] Configuration management
- [ ] Package export/import and download support
- [ ] Package groups, changelog, dependency inspection, disk-space tools
- [ ] GUI workflows for all supported features, progress reporting and cancellation
- [ ] Logging and structured error reporting parity
- [ ] Fedora RPM spec, desktop integration, uninstall behavior
- [ ] Automated tests on Fedora Workstation and a Fedora Everything installation
- [ ] Compare outputs and behavior against the Rust implementation
- [ ] Review license and project documentation before final cutover

## Safety and testing
Do not merge this draft as a replacement yet. Test mutating operations only in disposable Fedora virtual machines until transaction handling, privilege behavior, and failure recovery are reviewed.
