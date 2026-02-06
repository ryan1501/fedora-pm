Name:           fedora-gaming-meta
Version:        1.2
Release:        1%{?dist}
Summary:        Flexible gaming meta-package for Fedora
License:        MIT
BuildArch:      noarch

# --- Hard Dependencies (Essential Core) ---
# These must be in the standard Fedora or RPM Fusion repos
Requires:       steam
Requires:       gamemode
Requires:       mangohud
Requires:       wine-core

# --- Soft Dependencies (The "Nice to Haves") ---
# DNF will try to install these, but won't fail if they are missing
Recommends:     lutris
Recommends:     bottles
Recommends:     heroic-games-launcher-bin
Recommends:     prism-launcher
Recommends:     discord
Recommends:     obs-studio
Recommends:     goverlay
Recommends:     piper
Recommends:     lact
Recommends:     protontricks

%description
A flexible meta-package for Fedora gaming. Uses soft dependencies 
(Recommends) for third-party launchers and tools to ensure 
installation succeeds even if specific repositories are missing.

%prep
%build
%install
%files
%changelog
* Tue Dec 30 2025 Your Name <youremail@example.com> - 1.2-1
- Switched most packages to Recommends for better flexibility.
