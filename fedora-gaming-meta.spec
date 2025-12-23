%define name fedora-gaming-meta
%define version 1.0.0
%define release 1%{?dist}

Summary: Meta-package for comprehensive gaming setup on Fedora
Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv3+
Group: Applications/Games
BuildArch: noarch
Source0: %{name}-%{version}.tar.gz

# Core gaming platforms and launchers
Requires: steam
Requires: lutris
Requires: wine
Requires: winetricks

# Performance optimization tools
Requires: gamemode
Requires: gamescope
Requires: mangohud

# Graphics APIs and translation layers
Requires: vulkan-loader
Requires: vulkan-tools
Requires: mesa-vulkan-drivers
# DXVK and VKD3D (from RPM Fusion - requires RPM Fusion Nonfree repository)
# Note: Enable RPM Fusion repositories before installation: ./enable-repos.sh
Requires: dxvk
Requires: vkd3d
# 32-bit Vulkan drivers (optional, but recommended for Wine compatibility)
# Note: May not be available on all systems - moved to Suggests
# Prefer not to fail the transaction if i686 variants are unavailable
Suggests: mesa-vulkan-drivers.i686
Suggests: dxvk.i686
Suggests: vkd3d.i686

# Wine dependencies for better compatibility
# Note: These require RPM Fusion Nonfree repository
Requires: wine-dxvk
# wine-vkd3d may not be available in all repositories - moved to Suggests below

# Audio for gaming
Requires: pipewire-pulseaudio
Requires: pipewire-alsa
Requires: pipewire-jack-audio-connection-kit

# Gaming fonts (better Unicode support)
Requires: gdouros-symbola-fonts
# Note: Package name may vary - try google-noto-fonts-common or google-noto-sans-fonts
# If not available, install manually: sudo dnf install google-noto-fonts-common
Suggests: google-noto-fonts-common
Requires: google-noto-emoji-fonts

# Input devices and controllers
Requires: jstest-gtk
Requires: antimicrox

# System utilities for gaming
# Note: gamemode-daemon, libgamemode, and libgamemodeauto are typically provided by the gamemode package
# If they're separate packages in your distribution, uncomment these:
# Requires: gamemode-daemon
# Requires: libgamemode
# Requires: libgamemodeauto

# Optional packages that may not be available in all repositories
# These are moved to Suggests so installation doesn't fail if they're unavailable
# Install these manually if needed after enabling RPM Fusion repositories
Suggests: mesa-vulkan-drivers.i686
Suggests: gamemode-daemon
Suggests: libgamemode
Suggests: libgamemodeauto
Suggests: wine-vkd3d

# Other optional but recommended
Suggests: obs-studio
Suggests: discord
Suggests: goverlay
Suggests: proton-ge-custom
Suggests: bottles

%description
Fedora Gaming Meta is a comprehensive meta-package that installs all essential
gaming tools and libraries for a complete gaming experience on Fedora Linux.

This package includes:
- Steam - Digital game distribution platform
- Lutris - Open gaming platform for Linux
- Wine - Compatibility layer for running Windows games
- Winetricks - Helper script for Wine
- GameMode - Optimize system performance for games
- GameScope - Micro-compositor for games (Steam Deck-like experience)
- MangoHud - Vulkan overlay for monitoring FPS, temperatures, etc.
- DXVK/VKD3D - Vulkan-based translation layers for DirectX
- Mesa Vulkan drivers - Open-source Vulkan drivers (64-bit and 32-bit)
- Gaming fonts - Better Unicode and emoji support
- Controller support tools - Gamepad configuration utilities
- Audio stack - PipeWire for low-latency audio

After installation, you'll have everything needed to play games on Fedora,
including Windows games through Wine/Proton, native Linux games, and emulators.

%prep
%setup -q

%build
# Meta-package, nothing to build

%install
# Meta-package, no files to install
# All dependencies are pulled in via Requires
# Documentation is handled automatically by %doc directive in %files section

%files
# Meta-package has no files of its own
%doc fedora-gaming-meta-README.md

%changelog
* Wed Jan 01 2025 Fedora Gaming Meta <packager@example.com> - 1.0.0-1
- Initial release
- Includes Steam, Lutris, Wine, GameMode, GameScope, MangoHud, DXVK, and more
- Added Mesa Vulkan drivers (64-bit and 32-bit) for better compatibility
- Added GameScope micro-compositor for enhanced gaming experience
- Comprehensive gaming setup for Fedora Linux

