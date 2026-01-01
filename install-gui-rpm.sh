#!/bin/bash

# Install GUI RPM with dependency fixes

echo "Installing fedora-pm-gui RPM..."

# First, install available dependencies
echo "Installing available dependencies..."
sudo dnf install -y python3-pyside6

# Check if fedora-pm CLI exists, install from source if needed
if ! command -v fedora-pm &> /dev/null; then
    echo "fedora-pm CLI not found, building from source..."
    cargo build --release
    sudo cp target/release/fedora-pm /usr/local/bin/
    sudo chmod +x /usr/local/bin/fedora-pm
fi

# Install GUI RPM with --skip-broken to ignore missing subpackages
echo "Installing GUI RPM..."
sudo dnf install --skip-broken -y rpmbuild/RPMS/noarch/fedora-pm-gui-1.0.0-1.fc43.noarch.rpm

# Install desktop file if not already installed
if [ ! -f /usr/share/applications/fedora-pm.desktop ]; then
    echo "Installing desktop entry..."
    sudo cp fedora-pm.desktop /usr/share/applications/
    sudo update-desktop-database /usr/share/applications/
fi

echo "Installation complete!"
echo "Launch with: fedora-pm-gui"