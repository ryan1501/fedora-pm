# Fedora Gaming Meta - Quick Start Guide

## What is This?

`fedora-gaming-meta` is a meta-package (similar to `cachyos-gaming-meta`) that installs all essential gaming tools and libraries for Fedora Linux in one command.

## Quick Installation

### Prerequisites (enable RPM Fusion first)

1) Enable RPM Fusion (required for Steam, DXVK/VKD3D, some drivers)
```bash
# Free repository
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm

# Nonfree repository
sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
```

### Build and Install

1. **Build the RPM:**
```bash
./build-gaming-meta.sh
```

2. **Install the package:**
```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm
```

This will install all gaming packages automatically!

## What Gets Installed

### Core Gaming Platforms
- ✅ Steam
- ✅ Lutris  
- ✅ Wine + Winetricks

### Performance Tools
- ✅ GameMode (auto-optimizes system for games)
- ✅ MangoHud (FPS/temp overlay)

### Graphics APIs
- ✅ Vulkan (loader + tools)
- ✅ DXVK (DirectX → Vulkan, from RPM Fusion)
- ✅ VKD3D (DirectX 12 → Vulkan, from RPM Fusion)

### Audio
- ✅ PipeWire (low-latency audio)

### Gaming Fonts
- ✅ Unicode/emoji support fonts

### Controller Support
- ✅ Gamepad testing tools
- ✅ Controller mapping utilities

## After Installation

### Start Gaming!

**Steam:**
```bash
steam
```

**Lutris:**
```bash
lutris
```

**Run a Windows game with Wine:**
```bash
wine /path/to/game.exe
```

### Enable GameMode

GameMode activates automatically, but you can verify:
```bash
systemctl --user status gamemoded
```

### Use MangoHud Overlay

Monitor FPS and performance:
```bash
mangohud steam
# or
mangohud lutris
```

## GPU Drivers (Recommended)

For best performance, install GPU drivers:

**NVIDIA:**
```bash
sudo dnf install akmod-nvidia xorg-x11-drv-nvidia-cuda
sudo akmods --force
sudo reboot
```

**AMD/Intel:**
```bash
sudo dnf install mesa-vulkan-drivers mesa-dri-drivers
```

## Troubleshooting

### Package Not Found Errors

Some packages may require RPM Fusion. Make sure both repositories are enabled (see Prerequisites).

### Steam Issues

Steam requires RPM Fusion Nonfree. Verify it's enabled:
```bash
dnf repolist | grep rpmfusion
```

### Wine/Gaming Performance

1. Check GPU drivers are installed
2. Verify GameMode is running
3. Use MangoHud to monitor performance
4. Check WineHQ AppDB for game-specific fixes

## Optional Packages

The meta-package suggests (but doesn't require):
- `obs-studio` - Streaming/recording
- `discord` - Gaming chat
- `goverlay` - MangoHud GUI config
- `proton-ge-custom` - Enhanced Proton (install from COPR)
- `bottles` - Wine prefix manager

Install separately if needed:
```bash
sudo dnf install obs-studio discord
```

## Differences from cachyos-gaming-meta

- Uses Fedora package names
- Requires RPM Fusion (not needed on CachyOS)
- Uses standard Fedora audio stack
- Compatible with all Fedora versions

## Uninstall

To remove all gaming packages:
```bash
sudo dnf remove fedora-gaming-meta
```

Note: This removes the meta-package but keeps the installed packages. To remove everything:
```bash
sudo dnf remove steam lutris wine gamemode mangohud
# ... etc
```

## More Information

See `fedora-gaming-meta-README.md` for detailed documentation.

