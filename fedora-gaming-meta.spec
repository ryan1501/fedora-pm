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

# Core gaming platforms and launchers
Requires: steam
Requires: lutris
Requires: wine
Requires: winetricks

# Performance optimization tools
Requires: gamemode
Requires: mangohud

# Graphics APIs and translation layers
Requires: vulkan-loader
Requires: vulkan-tools
Requires: dxvk
Requires: vkd3d

# Wine dependencies for better compatibility
Requires: wine-dxvk
Requires: wine-vkd3d

# Audio for gaming
Requires: pipewire-pulseaudio
Requires: pipewire-alsa
Requires: pipewire-jack-audio-connection-kit

# Gaming fonts (better Unicode support)
Requires: gdouros-symbola-fonts
Requires: google-noto-fonts
Requires: google-noto-emoji-fonts

# Input devices and controllers
Requires: jstest-gtk
Requires: antimicrox

# System utilities for gaming
Requires: gamemode-daemon
Requires: libgamemode
Requires: libgamemodeauto

# Optional but recommended
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
- MangoHud - Vulkan overlay for monitoring FPS, temperatures, etc.
- DXVK/VKD3D - Vulkan-based translation layers for DirectX
- Gaming fonts - Better Unicode and emoji support
- Controller support tools - Gamepad configuration utilities
- Audio stack - PipeWire for low-latency audio

After installation, you'll have everything needed to play games on Fedora,
including Windows games through Wine/Proton, native Linux games, and emulators.

%prep
# Meta-package, no source to unpack

%build
# Meta-package, nothing to build

%install
# Meta-package, no files to install
# All dependencies are pulled in via Requires

%files
# Meta-package has no files of its own
%doc fedora-gaming-meta-README.md

%changelog
* Wed Jan 01 2025 Fedora Gaming Meta <packager@example.com> - 1.0.0-1
- Initial release
- Includes Steam, Lutris, Wine, GameMode, MangoHud, DXVK, and more
- Comprehensive gaming setup for Fedora Linux

