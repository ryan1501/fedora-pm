# RPM Spec File for Fedora Package Manager GUI
%define name fedora-pm-gui
%define version 2.0.0
%define release 1%{?dist}

Summary: Qt GUI front-end for Fedora Package Manager
Name: %{name}
Version: %{version}
Release: %{release}
License: GPLv3+
Group: Applications/System
BuildArch: noarch
Source0: %{name}-%{version}.tar.gz

# Python runtime dependencies
Requires: python3
Requires: python3-pyside6
Requires: python3-pyside6-qtwidgets
Requires: python3-pyside6-qtgui
Requires: python3-pyside6-qtcore

# System dependencies
Requires: fedora-pm >= %{version}
Requires: dnf
Requires: rpm

# Build dependencies
BuildRequires: python3-devel
BuildRequires: python3-setuptools
BuildRequires: desktop-file-utils
BuildRequires: libappstream-glib

%description
Fedora Package Manager GUI is a modern Qt-based graphical interface
for the fedora-pm command-line tool. It provides an easy-to-use
front-end for package management, system maintenance, security audits,
kernel management, driver installation, and gaming meta-package setup.

Features:
- Package installation, removal, and updates
- System health checks and maintenance
- Security audits and updates
- Kernel management and performance kernels
- Driver detection and installation (NVIDIA, etc.)
- Flatpak management
- Repository management
- Gaming meta-package installation
- Disk space analysis
- Package export/import functionality

%prep
%autosetup -n %{name}-%{version}

%build
# No compilation needed for Python script
# Create icon directory if needed
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/256x256/apps

%install
# Install the main GUI script
install -D -m 755 fedora-pm-gui.py %{buildroot}%{_bindir}/fedora-pm-gui

# Install the launcher script (fallback)
install -D -m 755 fedora-pm-gui-launcher.py %{buildroot}%{_bindir}/fedora-pm-gui-launcher

# Install desktop file
install -D -m 644 fedora-pm.desktop %{buildroot}%{_datadir}/applications/fedora-pm.desktop

# Install icon (using system icon as fallback)
# If you have a custom icon, add it here and install to:
# %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/fedora-pm.png

# Create metainfo directory
mkdir -p %{buildroot}%{_metainfodir}

# Install appstream metadata
cat > %{buildroot}%{_metainfodir}/fedora-pm-gui.metainfo.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>fedora-pm-gui</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-3.0+</project_license>
  <name>Fedora Package Manager</name>
  <summary>Modern package manager for Fedora Linux</summary>
  <description>
    <p>
      Fedora Package Manager GUI provides a modern Qt-based interface
      for managing packages, system maintenance, and gaming setup on Fedora Linux.
    </p>
  </description>
  <launchable type="desktop-id">fedora-pm.desktop</launchable>
  <provides>
    <binary>fedora-pm-gui</binary>
  </provides>
  <url type="homepage">https://github.com/fedora-pm/fedora-pm</url>
  <url type="bugtracker">https://github.com/fedora-pm/fedora-pm/issues</url>
  <categories>
    <category>System</category>
    <category>PackageManager</category>
    <category>Settings</category>
  </categories>
  <keywords>
    <keyword>package</keyword>
    <keyword>manager</keyword>
    <keyword>dnf</keyword>
    <keyword>rpm</keyword>
    <keyword>fedora</keyword>
    <keyword>install</keyword>
    <keyword>update</keyword>
    <keyword>remove</keyword>
  </keywords>
</component>
EOF

%check
# Verify desktop file
desktop-file-validate %{buildroot}%{_datadir}/applications/fedora-pm.desktop

# Verify appstream metadata
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/fedora-pm-gui.metainfo.xml

%files
%{_bindir}/fedora-pm-gui
%{_bindir}/fedora-pm-gui-launcher
%{_datadir}/applications/fedora-pm.desktop
%{_metainfodir}/fedora-pm-gui.metainfo.xml
# Uncomment if you have a custom icon:
# %{_datadir}/icons/hicolor/256x256/apps/fedora-pm.png

%doc README.md
# License file will be added if it exists

%changelog
* Tue Dec 30 2025 Fedora PM Maintainer <maintainer@fedora-pm.org> - 1.0.0-1
- Initial RPM release of fedora-pm-gui
- Qt-based GUI front-end for fedora-pm CLI tool
- Support for package management, system maintenance, security audits
- Kernel management, driver installation, and gaming setup
- Flatpak integration and repository management
- Appstream metadata and desktop integration