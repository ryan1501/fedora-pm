Name:           fedora-pm-gui
Version:        2.0.0
Release:        1%{?dist}
Summary:        Qt 6 graphical interface for Fedora Package Manager
License:        MIT OR Apache-2.0
%global forgeurl https://github.com/ryan1501/fedora-pm
URL:            %{forgeurl}
Source0:        fedora-pm-%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  qt6-qtbase-devel
BuildRequires:  desktop-file-utils

Requires:       dnf
Requires:       rpm
Requires:       polkit
Recommends:     flatpak

%description
A Qt 6 Widgets graphical interface for Fedora package management, including
DNF package operations, transaction confirmation, repository and security
inspection, and Flatpak actions.

%prep
%autosetup -n fedora-pm-%{version}

%build
%cmake -DFEDORAPM_BUILD_GUI=ON -DFEDORAPM_BUILD_TESTS=OFF
%cmake_build

%install
%cmake_install
rm -f %{buildroot}%{_bindir}/fedora-pm
install -D -m 0644 fedora-pm.desktop %{buildroot}%{_datadir}/applications/fedora-pm.desktop
install -D -m 0644 fedora-pm.metainfo.xml %{buildroot}%{_metainfodir}/fedora-pm.metainfo.xml

%check
desktop-file-validate fedora-pm.desktop

%files
%{_bindir}/fedora-pm-gui
%{_datadir}/applications/fedora-pm.desktop
%{_metainfodir}/fedora-pm.metainfo.xml
%doc README.md docs/CPP_MIGRATION_PLAN.md
