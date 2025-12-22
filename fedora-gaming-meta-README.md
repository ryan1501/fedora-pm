# Fedora Gaming Meta Package

A comprehensive meta-package that installs all essential gaming tools and libraries for Fedora Linux, similar to `cachyos-gaming-meta` but tailored for Fedora.

## What's Included

### Core Gaming Platforms
- **Steam** - Digital game distribution platform
- **Lutris** - Open gaming platform for managing and playing games
- **Wine** - Compatibility layer for running Windows applications and games
- **Winetricks** - Helper script for installing Windows components in Wine

### Performance Optimization
- **GameMode** - Automatically optimizes system performance when games are running
- **MangoHud** - Vulkan overlay for monitoring FPS, temperatures, GPU usage, etc.

### Graphics APIs & Translation Layers
- **Vulkan** - Modern graphics API (loader and tools)
- **DXVK** - Vulkan-based translation layer for DirectX 9/10/11
- **VKD3D** - Vulkan-based translation layer for DirectX 12
- **Wine-DXVK/VKD3D** - Wine integration for DXVK and VKD3D

### Audio Stack
- **PipeWire** - Modern audio server with PulseAudio and ALSA compatibility
- **PipeWire JACK** - Low-latency audio support for professional applications

### Gaming Fonts
- **Symbola Fonts** - Better Unicode symbol support
- **Google Noto Fonts** - Comprehensive font family with wide language support
- **Noto Emoji** - Emoji font support

### Input Devices
- **jstest-gtk** - Gamepad testing and configuration tool
- **AntimicroX** - Graphical program used to map keyboard keys and mouse controls to a gamepad

### Suggested Packages (Optional)
- **OBS Studio** - Streaming and recording software
- **Discord** - Gaming communication platform
- **GOverlay** - GUI for MangoHud configuration
- **Proton-GE-Custom** - Community-enhanced Proton for Steam
- **Bottles** - Easy-to-use Wine prefix manager

## Installation

### Build and Install from Source

1. Install build dependencies:
```bash
sudo dnf install rpm-build rpmdevtools
```

2. Set up RPM build tree (if not already done):
```bash
rpmdev-setuptree
```

3. Build the package:
```bash
./build-gaming-meta.sh
```

4. Install the built RPM:
```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm
```

### Manual Build

1. Create source tarball (meta-package doesn't need source, but RPM requires it):
```bash
mkdir -p ~/rpmbuild/SOURCES
tar -czf ~/rpmbuild/SOURCES/fedora-gaming-meta-1.0.0.tar.gz \
    fedora-gaming-meta.spec \
    fedora-gaming-meta-README.md
```

2. Copy spec file:
```bash
cp fedora-gaming-meta.spec ~/rpmbuild/SPECS/
```

3. Build RPM:
```bash
cd ~/rpmbuild/SPECS
rpmbuild -ba fedora-gaming-meta.spec
```

4. Install:
```bash
sudo dnf install ~/rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm
```

## Usage

After installation, all gaming tools will be available:

### Steam
```bash
steam
```

### Lutris
```bash
lutris
```

### Wine
```bash
wine /path/to/game.exe
```

### GameMode
GameMode automatically activates when games are launched. You can also enable it manually:
```bash
gamemoderun /path/to/game
```

### MangoHud
Enable MangoHud overlay for any Vulkan application:
```bash
mangohud /path/to/game
```

Or for OpenGL:
```bash
mangohud --dlsym /path/to/game
```

### Winetricks
Install Windows components:
```bash
winetricks d3dcompiler_47
winetricks vcrun2019
```

## Post-Installation Setup

### Enable RPM Fusion Repositories

Some packages may require RPM Fusion repositories:

```bash
# Enable RPM Fusion Free
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm

# Enable RPM Fusion Nonfree
sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
```

### Install GPU Drivers

For optimal gaming performance, install appropriate GPU drivers:

**NVIDIA:**
```bash
sudo dnf install akmod-nvidia xorg-x11-drv-nvidia-cuda
```

**AMD:**
```bash
sudo dnf install mesa-vulkan-drivers mesa-dri-drivers
```

**Intel:**
```bash
sudo dnf install mesa-vulkan-drivers mesa-dri-drivers
```

### Configure GameMode

GameMode should work automatically, but you can verify it's installed:
```bash
systemctl --user status gamemoded
```

### Configure MangoHud

Create a config file at `~/.config/MangoHud/MangoHud.conf`:
```ini
# Example MangoHud configuration
fps_limit=0
toggle_hud=F12
```

## Troubleshooting

### Missing Packages

If some packages are not found, you may need to enable additional repositories:
- **RPM Fusion** - For Steam, some Wine packages, and proprietary codecs
- **COPR** - For community packages like Proton-GE

### Wine Issues

If games don't run properly in Wine:
1. Use Winetricks to install required Windows components
2. Check WineHQ AppDB for game-specific instructions
3. Consider using Lutris, which handles Wine configuration automatically

### Performance Issues

1. Ensure GameMode is running: `systemctl --user status gamemoded`
2. Check GPU drivers are installed correctly
3. Use MangoHud to monitor system performance
4. Consider using a gaming kernel (like CachyOS kernels)

### Audio Issues

If you experience audio latency or issues:
1. Ensure PipeWire is running: `systemctl --user status pipewire`
2. Configure PipeWire for low latency if needed
3. Check audio device settings in your game

## Differences from cachyos-gaming-meta

This package is tailored for Fedora and uses:
- Fedora's native package names
- RPM Fusion repositories where needed
- Standard Fedora audio stack (PipeWire)
- Fedora-compatible gaming tools

Some CachyOS-specific optimizations (like CachyOS kernels) are not included but can be installed separately.

## Contributing

To add or remove packages from this meta-package, edit `fedora-gaming-meta.spec` and update the `Requires:` section.

## License

GPLv3+

