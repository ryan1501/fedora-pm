Name:           fedora-pm
Version:        2.0.0
Release:        1%{?dist}
Summary:        Fedora package management CLI
License:        MIT OR Apache-2.0
URL:            https://github.com/ryan1501/fedora-pm
Source0:        %{url}/archive/refs/tags/v%{version}.tar.gz

BuildRequires:  cmake
BuildRequires:  gcc-c++
BuildRequires:  make

Requires:       dnf
Requires:       dnf-plugins-core
Requires:       rpm
Requires:       polkit

%description
A C++20 command-line package management tool for Fedora Linux. It provides
DNF and Flatpak operations, package export/import, security and system
diagnostics, and local operation history.

%prep
%autosetup -n fedora-pm-%{version}

%build
%cmake -DFEDORAPM_BUILD_GUI=OFF -DFEDORAPM_BUILD_TESTS=ON
%cmake_build

%check
%ctest --output-on-failure

%install
%cmake_install

%files
%{_bindir}/fedora-pm
%doc README.md docs/CPP_MIGRATION_PLAN.md
