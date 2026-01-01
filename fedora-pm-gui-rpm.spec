Name:           fedora-pm-gui
Version:        1.0.0
Release:        1%{?dist}
Summary:        Qt-based GUI for Fedora Package Manager
License:        MIT
URL:            https://github.com/yourusername/fedora-pm
Source0:        %{name}-%{version}.tar.gz

BuildArch:      noarch

# Build requirements
BuildRequires:  python3-devel
BuildRequires:  python3-setuptools
BuildRequires:  desktop-file-utils
BuildRequires:  libappstream-glib

# Runtime requirements
Requires:       python3 >= 3.8
Requires:       python3-pyside6
Requires:       fedora-pm >= 1.0.0
Requires:       dnf
Requires:       rpm

%description
A modern Qt-based graphical interface for the Fedora Package Manager.
Provides an easy-to-use GUI for package management, system health checks,
security audits, and system maintenance tasks.

Features:
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
# No compilation needed - Python scripts

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/metainfo
mkdir -p %{buildroot}%{_docdir}/%{name}

# Install main GUI script
install -m 0755 fedora-pm-gui.py %{buildroot}%{_bindir}/fedora-pm-gui

# Install GUI launcher
install -m 0755 fedora-pm-gui-launcher.py %{buildroot}%{_bindir}/fedora-pm-gui-launcher

# Install desktop file
install -m 0644 fedora-pm.desktop %{buildroot}%{_datadir}/applications/

# Install appstream metadata
cat > %{buildroot}%{_datadir}/metainfo/fedora-pm-gui.metainfo.xml << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<component type="desktop-application">
  <id>fedora-pm.desktop</id>
  <metadata_license>MIT</metadata_license>
  <project_license>MIT</project_license>
  <name>Fedora Package Manager</name>
  <summary>Graphical package manager for Fedora Linux</summary>
  <description>
    <p>A modern Qt-based graphical interface for managing packages on Fedora Linux.</p>
  </description>
  <launchable type="desktop-id">fedora-pm.desktop</launchable>
  <provides>
    <binary>fedora-pm-gui</binary>
  </provides>
  <url type="homepage">https://github.com/yourusername/fedora-pm</url>
  <categories>
    <category>System</category>
    <category>PackageManager</category>
  </categories>
</component>
EOF

# Install documentation
install -m 0644 README.md %{buildroot}%{_docdir}/%{name}/
install -m 0644 requirements.txt %{buildroot}%{_docdir}/%{name}/

%files
%{_bindir}/fedora-pm-gui
%{_bindir}/fedora-pm-gui-launcher
%{_datadir}/applications/fedora-pm.desktop
%{_datadir}/metainfo/fedora-pm-gui.metainfo.xml
%doc %{_docdir}/%{name}/*

%changelog
* Wed Jan 01 2025 Fedora Package Manager <packager@example.com> - 1.0.0-1
- Initial GUI release with Qt interface
- Package management and system tools
- Desktop integration with appstream metadata