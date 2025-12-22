%define name fedora-pm
%define version 1.0.0
%define release 1%{?dist}

Summary: Modern package manager for Fedora Linux with GUI
Name: %{name}
Version: %{version}
Release: %{release}
License: MIT
Group: Applications/System
Source0: %{name}-%{version}.tar.gz
BuildArch: noarch
Requires: python3 >= 3.6
Requires: python3-pyside6
Requires: dnf
Requires: rpm
Requires: sudo
BuildRequires: python3-devel
BuildRequires: python3-pyside6

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

%prep
%setup -q

%build
# No build step needed for Python scripts

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/applications
mkdir -p %{buildroot}%{_datadir}/%{name}
mkdir -p %{buildroot}%{_mandir}/man1

# Install CLI script
install -m 755 fedora-pm.py %{buildroot}%{_bindir}/fedora-pm

# Install GUI script
install -m 755 fedora-pm-gui.py %{buildroot}%{_bindir}/fedora-pm-gui

# Install module
install -m 644 fedora_pm.py %{buildroot}%{_datadir}/%{name}/

# Install desktop entry
install -m 644 fedora-pm.desktop %{buildroot}%{_datadir}/applications/

# Install man page if exists
if [ -f fedora-pm.1 ]; then
    install -m 644 fedora-pm.1 %{buildroot}%{_mandir}/man1/
fi

%files
%{_bindir}/fedora-pm
%{_bindir}/fedora-pm-gui
%{_datadir}/%{name}/fedora_pm.py
%{_datadir}/applications/fedora-pm.desktop
%doc README.md
%doc LICENSE
%{_mandir}/man1/fedora-pm.1*

%changelog
* Wed Jan 01 2025 Fedora Package Manager <packager@example.com> - 1.0.0-1
- Initial release with CLI and GUI interfaces
- Package, kernel, and driver management features

