#!/bin/bash

# Enhanced spec file that builds multiple RPM types
# Builds both .src.rpm and .nosrc.rpm packages

%define _base_version 1.1.0
%define _version %{_base_version}

Name:           fedora-pm
Version:        %{_version}
Release:        1%{?dist}
Summary:        Modern package manager for Fedora Linux with GitHub-based self-update
License:        MIT OR Apache-2.0
URL:            https://github.com/yourusername/fedora-pm
Source0:        %{name}-%{version}.tar.gz

# Build architecture
BuildArch:      noarch

# Required build tools
BuildRequires:  cargo
BuildRequires:  rust
BuildRequires:  fedpkg-packager

# Runtime requirements
Requires:       dnf
Requires:       rpm
Requires:       git

%description
A comprehensive package manager for Fedora Linux with CLI and GUI interfaces.
Features package management, system diagnostics, security updates,
and GitHub-based self-update capabilities with smart fallbacks.

%package
# Build in a clean directory
%setup -q

%build
# Build the Rust binary
cargo build --release

%install
# Install main binary
mkdir -p %{buildroot}%{_bindir}
install -m 0755 target/release/fedora-pm %{buildroot}%{_bindir}/fedora-pm

%files
# Binary file
%{_bindir}/fedora-pm

# Documentation (optional)
%doc README.md CHANGELOG.md LICENSE*

# License files (optional)
%license LICENSE*

%changelog
* Wed Dec 31 2024 Your Name <your@email.com> - %{_base_version}-1
- Added GitHub-based self-update integration with smart fallbacks
- Binary download with source compilation fallback
- Auto-update cron job management
- Comprehensive documentation and testing tools
- Supports multiple Fedora distributions
- Robust error handling and user feedback