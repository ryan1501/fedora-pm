Name:           fedora-pm-gui
Version:        1.1.0
Release:        1%{?dist}
Summary:        Native GUI for Fedora Package Manager
License:        MIT OR Apache-2.0
URL:            https://github.com/fedora-pm/fedora-pm
Source0:        %{name}-%{version}.tar.gz

BuildArch:      x86_64

# Build requirements
BuildRequires:  cargo
BuildRequires:  rust
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

# Runtime requirements
Requires:       fedora-pm >= 1.1.0
Requires:       dnf
Requires:       rpm

%description
A modern native graphical interface for Fedora Package Manager.
Provides an easy-to-use GUI for package management, system health checks,
security audits, and system maintenance tasks.

Features:
- Native Rust implementation for better performance
- Package management (install, remove, update, search)
- System health check (doctor)
- Security audits
- Flatpak integration
- Backup/restore packages
- Repository management
- Disk space analysis
- Gaming meta package installation

%prep
%setup -q

%build
# Build the GUI binary
cargo build --release --bin fedora-pm-gui

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/metainfo
mkdir -p %{buildroot}%{_docdir}/%{name}

# Install GUI binary
install -m 0755 target/release/fedora-pm-gui %{buildroot}%{_bindir}/fedora-pm-gui

# Install desktop file
cat > %{buildroot}%{_datadir}/applications/fedora-pm-gui.desktop << 'EOF'
[Desktop Entry]
Name=Fedora Package Manager
Comment=Graphical package manager for Fedora Linux
Exec=fedora-pm-gui
Icon=fedora-pm
Terminal=false
Type=Application
Categories=System;PackageManager;
StartupNotify=true
EOF

# Install appstream metadata
cat > %{buildroot}%{_datadir}/metainfo/fedora-pm-gui.metainfo.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>fedora-pm-gui.desktop</id>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT OR Apache-2.0</project_license>
  <name>Fedora Package Manager</name>
  <summary>Graphical package manager for Fedora Linux</summary>
  <description>
    <p>A modern native Rust-based graphical interface for managing packages on Fedora Linux.</p>
  </description>
  <launchable type="desktop-id">fedora-pm-gui.desktop</launchable>
  <provides>
    <binary>fedora-pm-gui</binary>
  </provides>
  <url type="homepage">https://github.com/fedora-pm/fedora-pm</url>
  <categories>
    <category>System</category>
    <category>PackageManager</category>
  </categories>
</component>
EOF

# Install documentation
install -m 0644 README.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 Cargo.toml %{buildroot}%{_docdir}/%{name}/

%files
%{_bindir}/fedora-pm-gui
%{_datadir}/applications/fedora-pm-gui.desktop
%{_datadir}/metainfo/fedora-pm-gui.metainfo.xml
%doc %{_docdir}/%{name}/*

%changelog
* Thu Jan 01 2026 Fedora Package Manager <packager@example.com> - 1.1.0-1
- Complete rewrite in native Rust
- Removed Python and Qt dependencies
- Better performance and smaller footprint
- Improved security and reliability