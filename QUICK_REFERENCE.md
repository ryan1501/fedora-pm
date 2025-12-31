# Fedora-PM Quick Reference

Quick command reference for fedora-pm.

## Package Management

| Command | Description |
|---------|-------------|
| `fedora-pm install <pkg>` | Install package(s) |
| `fedora-pm remove <pkg>` | Remove package(s) |
| `fedora-pm update` | Update all packages |
| `fedora-pm update <pkg>` | Update specific package(s) |
| `fedora-pm search <query>` | Search for packages |
| `fedora-pm info <pkg>` | Show package information |
| `fedora-pm list installed` | List installed packages |
| `fedora-pm list available` | List available packages |
| `fedora-pm clean` | Clean cache and metadata |

## History & Rollback

| Command | Description |
|---------|-------------|
| `fedora-pm history` | Show operation history |
| `fedora-pm history -n 20` | Show last 20 operations |
| `fedora-pm rollback` | Undo last operation |
| `fedora-pm rollback --id 5` | Rollback specific operation |

## Dependencies

| Command | Description |
|---------|-------------|
| `fedora-pm deps <pkg> --tree` | Show dependency tree |
| `fedora-pm deps <pkg> --reverse` | Show reverse dependencies |

## Package Groups

| Command | Description |
|---------|-------------|
| `fedora-pm group list` | List all groups |
| `fedora-pm group info <group>` | Show group information |
| `fedora-pm group install <group>` | Install package group |
| `fedora-pm group remove <group>` | Remove package group |

## System Health

| Command | Description |
|---------|-------------|
| `fedora-pm doctor` | Run system health check |
| `fedora-pm size --analyze` | Analyze disk space |
| `fedora-pm size --top 20` | Show 20 largest packages |
| `fedora-pm clean-orphans` | Remove orphaned packages |

## Flatpak

| Command | Description |
|---------|-------------|
| `fedora-pm flatpak setup-flathub` | Setup Flathub repository |
| `fedora-pm flatpak search <query>` | Search Flatpak apps |
| `fedora-pm flatpak install <app>` | Install Flatpak app |
| `fedora-pm flatpak list` | List installed Flatpaks |
| `fedora-pm flatpak update` | Update all Flatpaks |
| `fedora-pm flatpak remove <app>` | Remove Flatpak app |

## Backup & Restore

| Command | Description |
|---------|-------------|
| `fedora-pm export <file>` | Export package list |
| `fedora-pm export <file> --with-flatpak` | Export with Flatpaks |
| `fedora-pm import <file>` | Import and install packages |

## Repositories

| Command | Description |
|---------|-------------|
| `fedora-pm repo list` | List enabled repositories |
| `fedora-pm repo list --all` | List all repositories |
| `fedora-pm repo enable <repo>` | Enable repository |
| `fedora-pm repo disable <repo>` | Disable repository |
| `fedora-pm repo add <name> <url>` | Add repository |
| `fedora-pm repo remove <repo>` | Remove repository |
| `fedora-pm repo refresh` | Refresh metadata |

## Security

| Command | Description |
|---------|-------------|
| `fedora-pm security check` | Check for security updates |
| `fedora-pm security audit` | Full security audit |
| `fedora-pm security update` | Install security updates |
| `fedora-pm security list` | List security updates |
| `fedora-pm security cve <CVE>` | Check specific CVE |

## Download & Offline

| Command | Description |
|---------|-------------|
| `fedora-pm download <pkg>` | Download package(s) |
| `fedora-pm download <pkg> --dest <dir>` | Download to directory |
| `fedora-pm download <pkg> --with-deps` | Download with dependencies |
| `fedora-pm install-offline <rpm>` | Install downloaded RPMs |

## Changelog

| Command | Description |
|---------|-------------|
| `fedora-pm changelog <pkg>` | View package changelog |
| `fedora-pm changelog <pkg> -n 5` | Show last 5 entries |
| `fedora-pm whatsnew` | Show changelogs for updates |

## Kernel Management

| Command | Description |
|---------|-------------|
| `fedora-pm kernel current` | Show current kernel |
| `fedora-pm kernel list` | List installed kernels |
| `fedora-pm kernel list --available` | List available kernels |
| `fedora-pm kernel install` | Install latest kernel |
| `fedora-pm kernel remove <ver>` | Remove kernel version |
| `fedora-pm kernel remove-old` | Remove old kernels |
| `fedora-pm kernel info` | Show kernel information |

### CachyOS Kernels

| Command | Description |
|---------|-------------|
| `fedora-pm kernel cachyos check-cpu` | Check CPU support |
| `fedora-pm kernel cachyos list` | List CachyOS kernels |
| `fedora-pm kernel cachyos enable gcc` | Enable GCC repo |
| `fedora-pm kernel cachyos enable lto` | Enable LTO repo |
| `fedora-pm kernel cachyos install` | Install default kernel |
| `fedora-pm kernel cachyos install lts` | Install LTS kernel |
| `fedora-pm kernel cachyos install rt` | Install RT kernel |

## Driver Management

| Command | Description |
|---------|-------------|
| `fedora-pm driver status` | Show driver status |
| `fedora-pm driver detect` | Detect GPU hardware |
| `fedora-pm driver install-nvidia` | Install Nvidia drivers |
| `fedora-pm driver install-nvidia --cuda` | Install with CUDA |
| `fedora-pm driver check-nvidia` | Check Nvidia status |
| `fedora-pm driver remove-nvidia` | Remove Nvidia drivers |
| `fedora-pm driver list-nvidia` | List Nvidia drivers |
| `fedora-pm driver install-cuda` | Install CUDA toolkit |

## Gaming

| Command | Description |
|---------|-------------|
| `fedora-pm gaming install` | Install gaming meta package |
| `./enable-repos.sh` | Enable required repositories |

## Global Options

| Option | Description |
|--------|-------------|
| `-y, --yes` | Skip confirmation prompts |
| `-v` | Verbose output (warnings) |
| `-vv` | Very verbose (info) |
| `-vvv` | Debug output |
| `--quiet` | Minimal output |
| `--help` | Show help message |

## Common Workflows

### Daily Maintenance
```bash
fedora-pm update -y
fedora-pm clean
```

### Weekly Check
```bash
fedora-pm doctor
fedora-pm security audit
```

### Before Major Update
```bash
fedora-pm export backup.txt --with-flatpak
fedora-pm whatsnew
fedora-pm update -y
```

### System Migration
```bash
# Old system
fedora-pm export my-packages.txt --with-flatpak

# New system
fedora-pm import my-packages.txt -y
```

### Gaming Setup
```bash
./enable-repos.sh
fedora-pm gaming install -y
```

### Troubleshooting
```bash
fedora-pm doctor
fedora-pm deps <package> --tree
fedora-pm -vv install <package>
```

### Free Up Space
```bash
fedora-pm size --analyze
fedora-pm clean-orphans -y
fedora-pm clean
fedora-pm kernel remove-old --keep 2
```

## Tips

- Add `-y` to skip confirmations in scripts
- Use `--quiet` for cron jobs
- Use `-vv` when troubleshooting
- Export packages before major changes
- Run `doctor` regularly for system health
- Check `whatsnew` before updating
- Use `rollback` if something goes wrong

## See Also

- [README.md](README.md) - Full documentation
- [FEATURES.md](FEATURES.md) - Detailed feature guide
- `fedora-pm --help` - Built-in help
- `fedora-pm <command> --help` - Command-specific help
