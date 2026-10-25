# Fedora Package Manager (C++ migration)

Fedora Package Manager (FedoraAPM) is being migrated from the original Rust implementation to **C++20 with CMake and an optional Qt 6 Widgets GUI**. The migration is tracked on the `migration/cpp20-qt6` branch and in [`docs/CPP_MIGRATION_PLAN.md`](docs/CPP_MIGRATION_PLAN.md).

> **Migration status:** in progress. GitHub Actions is configured to build the C++ CLI, GUI, and tests on Ubuntu and Fedora containers and run unit tests and CLI smoke tests. Check the linked workflow for the latest result. Even a passing container job does not mean feature parity is complete or that Fedora Workstation and Fedora Everything have been tested on real installations. Keep the original Rust implementation as the reference until the migration checklist is complete and verified.

## Current C++ implementation

The C++ CLI currently includes these areas:

- DNF package search, info, installed/available lists, install/remove/update, and package-group operations
- Dependency and reverse-dependency queries
- DNF transaction history and undo operations
- Flatpak search, list, info, install/remove/update, and Flathub setup
- Repository list/info/enable/disable/add/remove/refresh
- Kernel listing, install/remove, and old-kernel cleanup
- Security update/advisory/CVE queries and security updates
- Package changelog queries
- Package-list export/import, including Flatpak IDs with `--with-flatpak` and legacy Rust-format backup entries
- Disk/package-size analysis, orphan cleanup, package downloads, and offline RPM installation
- GPU/NVIDIA/CUDA operations, gaming meta-package installation, CachyOS-related commands
- JSON configuration, local operation history, system information, and read-only diagnostics

The Qt GUI currently provides DNF search, package info, installed/available lists, install/remove/update with a transaction preview and explicit confirmation, DNF history, repository listing, security update listing, and Flatpak search/list/info plus confirmed install/remove/update actions. It is an early migration UI, not yet a replacement for every workflow in the original application.

## Build from source

### Fedora (Workstation or Everything)

Install the C++ build tools and Qt 6 development headers:

```bash
sudo dnf install cmake gcc-c++ qt6-qtbase-devel
```

On a minimal Fedora Everything installation, add the runtime tools used by repository-management and download commands, plus `pkexec` for GUI mutations:

```bash
sudo dnf install dnf-plugins-core polkit
# Optional: needed only for Flatpak commands
sudo dnf install flatpak
```

When run as a non-root user, the C++ CLI uses `sudo --non-interactive` for system-changing DNF operations. Configure non-interactive sudo authorization for those commands; they fail rather than prompting for a sudo password. The GUI uses `pkexec` for DNF transaction preview and application.

From the repository root:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=ON -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

If Qt 6 Widgets is unavailable, CMake can still build the CLI and tests by disabling the GUI:

```bash
cmake -S . -B build -DFEDORAPM_BUILD_GUI=OFF -DFEDORAPM_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The built executables are `build/fedora-pm` and, when Qt 6 Widgets is found, `build/fedora-pm-gui`.

### Ubuntu CI/development dependencies

```bash
sudo apt-get update
sudo apt-get install cmake g++ qt6-base-dev
```

Use the same CMake configure/build/test commands shown above.

## Usage

Start with the built CLI help or check the compiled version (`--version`, `-V`, or `version`):

```bash
./build/fedora-pm --version
./build/fedora-pm help
./build/fedora-pm info
./build/fedora-pm search firefox
./build/fedora-pm list --installed
./build/fedora-pm flatpak-list
./build/fedora-pm repo-list
./build/fedora-pm security-check
./build/fedora-pm export packages.txt
./build/fedora-pm export --with-flatpak packages-with-flatpaks.txt
```

The command list in `fedora-pm help` is the source of truth for the current C++ CLI. Commands that change system state should be tried only in a disposable Fedora virtual machine until transaction handling and failure recovery have been reviewed.

Launch the GUI from the build directory:

```bash
./build/fedora-pm-gui
```

The GUI needs a working graphical session and Qt 6 Widgets. DNF mutation actions use `pkexec` for a preview and ask for confirmation before applying the transaction. Read-only DNF and Flatpak actions do not require a mutation confirmation.

## Configuration and history

Configuration is stored under `~/.fedora-pm/config.json`. The default local operation history is `~/.fedora-pm/history.json`, unless `history_file` is configured. Compatibility with every legacy Rust configuration/history edge case is still being validated; keep backups of existing settings and history during testing.

## Known migration gaps

See the checklist in [`docs/CPP_MIGRATION_PLAN.md`](docs/CPP_MIGRATION_PLAN.md). Outstanding work includes:

- Audit remaining mutation paths and continue comparing history behavior with legacy Rust-generated files
- Real self-update functionality (the current C++ CLI only preserves placeholder command behavior)
- Exact CLI name, flag, output, and exit-code compatibility, including global flags (`--verbose`, `--quiet`, `--sudo`, `--config-dir`), package-specific updates, `deps --tree`, `size --total/--analyze`, and remaining aliases/options
- Broader GUI workflows and privilege/progress/cancellation testing
- Compatibility testing on Fedora Workstation and Fedora Everything
- CI builds both RPMs and tests install/uninstall in a disposable Fedora container; real Fedora Workstation/Everything testing and DNF plugin checks remain
- Behavior comparison against the Rust version, plus final documentation and license review

The repository `install.sh` now builds and installs the C++ implementation with CMake. Run `./install.sh --help` for `--cli`, `--gui`, `--both`, `--user`, `--prefix`, and `--dry-run` options. For development or manual control, use the CMake commands above.

## Legacy repository files

The repository still retains the Rust source and several older completion summaries, release notes, GUI guides, and packaging helpers as migration reference material. Those documents describe earlier Rust/Python behavior and are not authoritative for the C++ build. Use this README, `MIGRATION_STATUS.md`, `BUILD_RPM_QUICKSTART.md`, `RPM_BUILD_GUIDE.md`, and `docs/CPP_MIGRATION_PLAN.md` for current status. Legacy COPR upload scripts and `fedora-pm-copr.spec` are intentionally disabled on this branch so they cannot accidentally publish the old Rust application.

## Contributing and reporting issues

Work on the migration branch, add or update tests for behavior changes, and keep the migration checklist accurate. Do not use the draft migration as a replacement for the established implementation until the checklist is complete and the required Fedora testing has been performed.

Repository: https://github.com/ryan1501/fedora-pm
Migration pull request: https://github.com/ryan1501/fedora-pm/pull/14
