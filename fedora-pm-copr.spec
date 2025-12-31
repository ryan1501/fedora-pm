Name:           fedora-pm
Version:        1.1.0
Release:        1%{?dist}
Summary:        Modern package manager for Fedora Linux with GitHub-based self-update
License:        MIT OR Apache-2.0
URL:            https://github.com/yourusername/fedora-pm
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cargo rust
BuildArch:      noarch

Requires:       dnf
Requires:       rpm
Requires:       git

%description
A comprehensive package manager for Fedora Linux with CLI and GUI interfaces.
Features package management, system diagnostics, security updates,
and GitHub-based self-update capabilities with smart fallbacks.

%prep
%setup -q

%build
cargo build --release

%install
mkdir -p %{buildroot}%{_bindir}
install -m 0755 target/release/fedora-pm %{buildroot}%{_bindir}/fedora-pm

%files
%{_bindir}/fedora-pm
%doc README.md
%license LICENSE*

%changelog
* Wed Dec 31 2024 Your Name <your@email.com> - 1.1.0-1
- Added GitHub-based self-update integration
- Smart version checking from GitHub releases
- Binary download with source compilation fallback
- Auto-update cron job management
- Comprehensive documentation and testing tools