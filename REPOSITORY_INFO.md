# Repository Information for Fedora Gaming Meta

This document explains which repositories are needed for the packages in `fedora-gaming-meta.spec`.

## Required Repositories

### RPM Fusion (Required)

Most gaming packages come from **RPM Fusion** repositories:

- **RPM Fusion Free** - Open source gaming packages
- **RPM Fusion Nonfree** - Proprietary gaming packages (Steam, NVIDIA drivers, etc.)

#### Enable RPM Fusion:

```bash
# Automatic (recommended)
./enable-repos.sh

# Manual
FEDORA_VERSION=$(rpm -E %fedora)
sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-${FEDORA_VERSION}.noarch.rpm
sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-${FEDORA_VERSION}.noarch.rpm
```

### Fedora Official Repositories

Some packages are available in standard Fedora repositories:
- `vulkan-loader`, `vulkan-tools` - Usually in Fedora repos
- `pipewire-*` packages - Usually in Fedora repos
- `google-noto-fonts` - Usually in Fedora repos
- `gamemode`, `libgamemode*` - May be in Fedora or RPM Fusion

## Package Repository Mapping

### From RPM Fusion Nonfree:
- `steam` - Steam gaming platform
- `lutris` - Gaming platform
- `wine` - Windows compatibility layer
- `winetricks` - Wine helper
- `dxvk` - DirectX to Vulkan translation
- `vkd3d` - Direct3D 12 to Vulkan translation
- `wine-dxvk` - Wine DXVK integration
- `wine-vkd3d` - Wine VKD3D integration
- `mangohud` - Vulkan overlay

### From RPM Fusion Free or Fedora:
- `gamemode` - Performance optimization
- `gamemode-daemon` - GameMode daemon
- `libgamemode` - GameMode library
- `libgamemodeauto` - GameMode auto library

### From Fedora Official:
- `vulkan-loader` - Vulkan loader
- `vulkan-tools` - Vulkan tools
- `pipewire-pulseaudio` - Audio stack
- `pipewire-alsa` - ALSA support
- `pipewire-jack-audio-connection-kit` - JACK support
- `google-noto-fonts` - Fonts
- `google-noto-emoji-fonts` - Emoji fonts
- `gdouros-symbola-fonts` - Symbol fonts
- `jstest-gtk` - Joystick testing tool
- `antimicrox` - Controller mapping

## Quick Setup

1. **Enable repositories:**
   ```bash
   ./enable-repos.sh
   ```

2. **Check package availability:**
   ```bash
   ./check-package-repos.sh
   ```

3. **Install dependencies:**
   ```bash
   ./install-dependencies.sh
   ```

## Troubleshooting

### Package Not Found

If a package is not found after enabling repositories:

1. **Refresh repository cache:**
   ```bash
   sudo dnf makecache
   ```

2. **Check if repository is enabled:**
   ```bash
   dnf repolist enabled | grep rpmfusion
   ```

3. **Check package availability:**
   ```bash
   dnf repoquery --available <package-name>
   ```

4. **Check which repository provides the package:**
   ```bash
   dnf repoquery --available --qf "%{name} from %{repoid}" <package-name>
   ```

### Repository Issues

If RPM Fusion repositories fail to enable:

1. **Check Fedora version:**
   ```bash
   rpm -E %fedora
   cat /etc/fedora-release
   ```

2. **Try alternative mirror:**
   ```bash
   # Use a different mirror if the default fails
   sudo dnf install --setopt=fastestmirror=true \
     https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm
   ```

3. **Check network connectivity:**
   ```bash
   ping mirrors.rpmfusion.org
   ```

## Additional Notes

- Some packages may have different names in different Fedora versions
- Some packages may be split into multiple sub-packages
- Always ensure your system is up to date: `sudo dnf update`

