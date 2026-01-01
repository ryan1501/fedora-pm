%define name fedora-pm
%define version 1.1.0
%define release 1%{?dist}

Summary: Modern package manager for Fedora Linux with GUI
Name: %{name}
Version: %{version}
Release: %{release}
License: MIT OR Apache-2.0
Group: Applications/System
Source0: %{name}-%{version}.tar.gz
BuildArch: x86_64

# Build requirements
BuildRequires: cargo
BuildRequires: rust
BuildRequires: gcc
BuildRequires: make

# Runtime requirements
Requires: dnf
Requires: rpm
Requires: sudo

%description
Fedora Package Manager is a modern, user-friendly package manager for Fedora Linux
that provides both CLI and GUI interfaces for managing packages, kernels, and drivers.

Features:
- Install, remove, and update packages
- Search and list packages
- Kernel management (standard and CachyOS kernels)
- GPU driver management (Nvidia, AMD, Intel)
- System cache cleaning
- Operation history tracking
- Native Rust implementation with optional GUI

%prep
%setup -q

%build
cargo build --release

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/metainfo
mkdir -p %{buildroot}%{_mandir}/man1

# Install CLI binary
install -m 755 target/release/fedora-pm %{buildroot}%{_bindir}/

# Install GUI binary
install -m 755 target/release/fedora-pm-gui %{buildroot}%{_bindir}/

# Install desktop entry
cat > %{buildroot}%{_datadir}/applications/fedora-pm.desktop << 'EOF'
[Desktop Entry]
Name=Fedora Package Manager
Comment=Modern package manager for Fedora Linux
Exec=fedora-pm --gui
Icon=fedora-pm
Terminal=false
Type=Application
Categories=System;PackageManager;
StartupNotify=true
EOF

# Install appstream metadata
cat > %{buildroot}%{_datadir}/metainfo/fedora-pm.metainfo.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>fedora-pm.desktop</id>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT OR Apache-2.0</project_license>
  <name>Fedora Package Manager</name>
  <summary>Modern package manager for Fedora Linux</summary>
  <description>
    <p>A modern Rust-based package manager for Fedora Linux with CLI and GUI interfaces.</p>
  </description>
  <launchable type="desktop-id">fedora-pm.desktop</launchable>
  <provides>
    <binary>fedora-pm</binary>
    <binary>fedora-pm-gui</binary>
  </provides>
  <url type="homepage">https://github.com/fedora-pm/fedora-pm</url>
  <categories>
    <category>System</category>
    <category>PackageManager</category>
  </categories>
</component>
EOF

# Install man page if exists
if [ -f fedora-pm.1 ]; then
    install -m 644 fedora-pm.1 %{buildroot}%{_mandir}/man1/
fi

%files
%{_bindir}/fedora-pm
%{_bindir}/fedora-pm-gui
%{_datadir}/applications/fedora-pm.desktop
%{_datadir}/metainfo/fedora-pm.metainfo.xml
%doc README.md
%{_mandir}/man1/fedora-pm.1*

%changelog
* Thu Jan 01 2026 Fedora Package Manager <packager@example.com> - 1.1.0-1
- Complete rewrite in Rust for better performance
- Removed Python dependencies
- Native GUI implementation using Iced
- Improved security and reliability
- Reduced package size and dependencies