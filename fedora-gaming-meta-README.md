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
- **GameScope** - Micro-compositor for games, providing a Steam Deck-like experience with FSR upscaling, frame limiting, and more
- **MangoHud** - Vulkan overlay for monitoring FPS, temperatures, GPU usage, etc.

### Graphics APIs & Translation Layers
- **Vulkan** - Modern graphics API (loader and tools)
- **Mesa Vulkan Drivers** - Open-source Vulkan drivers for 64-bit and 32-bit applications (essential for Vulkan support)
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

### Prerequisites

Before building or installing, you should enable RPM Fusion repositories (required for most gaming packages):

**Option 1: Use the automated script (recommended):**
```bash
./enable-repos.sh
```

**Option 2: Manual setup:**
```bash
# Enable RPM Fusion Free
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm

# Enable RPM Fusion Nonfree
sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm
```

### Build and Install from Source

1. Install build dependencies:
```bash
sudo dnf install rpm-build rpmdevtools
```

2. Build the package:
```bash
./build-gaming-meta.sh
```

3. Install the built RPM:
```bash
sudo dnf install rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm
```

This will automatically install all required dependencies including Steam, Lutris, Wine, GameMode, GameScope, MangoHud, DXVK, Mesa Vulkan drivers, and more.

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

### GameScope
GameScope is a micro-compositor that provides a Steam Deck-like experience. Use it to run games with FSR upscaling, frame limiting, and more:

```bash
# Basic usage
gamescope -- /path/to/game

# With FSR upscaling (e.g., from 1080p to 4K)
gamescope -W 1920 -H 1080 -w 3840 -h 2160 -- /path/to/game

# With frame limiting (e.g., 60 FPS)
gamescope -r 60 -- /path/to/game

# Combined: FSR upscaling + frame limiting
gamescope -W 1920 -H 1080 -w 3840 -h 2160 -r 60 -- /path/to/game
```

GameScope is particularly useful for:
- Running games at lower resolution and upscaling with FSR
- Frame limiting for consistent performance
- Creating a dedicated gaming session
- Steam Deck-like experience on desktop

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

### Repository Management

The package includes helper scripts for repository management:

**Check which repositories provide packages:**
```bash
./check-package-repos.sh
```

**Enable required repositories:**
```bash
./enable-repos.sh
```

**Install dependencies directly (without building RPM):**
```bash
./install-dependencies.sh
```

### GPU Drivers

**Note:** Mesa Vulkan drivers (64-bit and 32-bit) are already included in this meta-package. However, for optimal performance, you may want to install vendor-specific drivers:

**NVIDIA:**
```bash
sudo dnf install akmod-nvidia xorg-x11-drv-nvidia-cuda
```

**AMD:**
The included Mesa Vulkan drivers should work well for AMD GPUs. For additional features:
```bash
sudo dnf install mesa-dri-drivers
```

**Intel:**
The included Mesa Vulkan drivers should work well for Intel GPUs. For additional features:
```bash
sudo dnf install mesa-dri-drivers
```

### Configure GameMode

GameMode should work automatically, but you can verify it's installed:
```bash
systemctl --user status gamemoded
```

### Configure GameScope

GameScope can be configured via command-line arguments or environment variables. Common options:
- `-W` / `-H`: Output resolution (what your display shows)
- `-w` / `-h`: Game render resolution (what the game renders at)
- `-r`: Frame rate limit
- `-f`: Fullscreen mode
- `-F fsr`: Enable FSR upscaling

Example configuration for a 4K display running games at 1080p with FSR:
```bash
gamescope -W 3840 -H 2160 -w 1920 -h 1080 -F fsr -r 60 -- /path/to/game
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

1. **Check repository status:**
   ```bash
   ./check-package-repos.sh
   ```

2. **Enable RPM Fusion repositories:**
   ```bash
   ./enable-repos.sh
   ```

3. **Refresh package cache:**
   ```bash
   sudo dnf makecache
   ```

**Common repositories needed:**
- **RPM Fusion Free/Nonfree** - For Steam, Wine, DXVK, VKD3D, MangoHud, GameScope, and more
- **COPR** - For community packages like Proton-GE-Custom

### Wine Issues

If games don't run properly in Wine:
1. Use Winetricks to install required Windows components
2. Check WineHQ AppDB for game-specific instructions
3. Consider using Lutris, which handles Wine configuration automatically

### Performance Issues

1. Ensure GameMode is running: `systemctl --user status gamemoded`
2. Check GPU drivers are installed correctly (Mesa Vulkan drivers are included)
3. Use MangoHud to monitor system performance
4. Try GameScope with FSR upscaling for better performance on high-resolution displays
5. Consider using a gaming kernel (like CachyOS kernels)
6. Verify Vulkan is working: `vulkaninfo` (should show your GPU)

### Audio Issues

If you experience audio latency or issues:
1. Ensure PipeWire is running: `systemctl --user status pipewire`
2. Configure PipeWire for low latency if needed
3. Check audio device settings in your game

## Package Contents Summary

This meta-package includes:

### Core Gaming Platforms
- Steam, Lutris, Wine, Winetricks

### Performance Tools
- GameMode (with daemon and libraries)
- GameScope (micro-compositor)
- MangoHud (Vulkan overlay)

### Graphics & APIs
- Vulkan loader and tools
- Mesa Vulkan drivers (64-bit and 32-bit)
- DXVK and VKD3D translation layers
- Wine-DXVK and Wine-VKD3D integration

### System Components
- PipeWire audio stack (PulseAudio, ALSA, JACK)
- Gaming fonts (Symbola, Noto, Emoji)
- Controller tools (jstest-gtk, AntimicroX)

### Optional (Suggested)
- OBS Studio, Discord, GOverlay, Proton-GE-Custom, Bottles

## Differences from cachyos-gaming-meta

This package is tailored for Fedora and uses:
- Fedora's native package names
- RPM Fusion repositories where needed
- Standard Fedora audio stack (PipeWire)
- Fedora-compatible gaming tools
- Includes Mesa Vulkan drivers (both 64-bit and 32-bit) for better compatibility
- Includes GameScope for Steam Deck-like experience

Some CachyOS-specific optimizations (like CachyOS kernels) are not included but can be installed separately.

## Contributing

To add or remove packages from this meta-package, edit `fedora-gaming-meta.spec` and update the `Requires:` section.

## License

GPLv3+

