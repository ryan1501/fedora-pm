# GUI Application Menu Fix

## Changes Made

The application wasn't showing up in the application menu due to a mismatch between the RPM spec file and the actual project structure. The following fixes have been applied:

### 1. Updated `fedora-pm-gui-rpm.spec`

**Previous Issues:**
- Spec file expected Python scripts that didn't exist
- Was set to `BuildArch: noarch` (incompatible with Rust binaries)
- Missing desktop database update scriptlets

**Fixes Applied:**
- ✅ Changed build requirements from Python to Rust/Cargo
- ✅ Removed `BuildArch: noarch` to allow architecture-specific binaries
- ✅ Updated build section to compile Rust binary with `cargo build --release --bin fedora-pm-gui`
- ✅ Updated install section to install the compiled binary from `target/release/fedora-pm-gui`
- ✅ Added `desktop-file-install` for proper desktop file installation
- ✅ Added `%post` scriptlet to run `update-desktop-database` after installation
- ✅ Added `%postun` scriptlet to run `update-desktop-database` after uninstallation
- ✅ Updated description to reflect Iced framework instead of Qt

### 2. Desktop File Validation

The existing `fedora-pm.desktop` file is valid with only a minor hint about extending categories. The file correctly:
- Points to `fedora-pm-gui` executable
- Has proper categories (System;PackageManager;)
- Includes appropriate metadata

## How to Rebuild and Install

### Option 1: Using rpmbuild directly

```bash
# Create tarball from source
cd /home/rblissett/fedora-pm
tar -czf rpmbuild/SOURCES/fedora-pm-1.0.0.tar.gz \
    --transform 's,^,fedora-pm-1.0.0/,' \
    src/ Cargo.toml Cargo.lock fedora-pm.desktop README.md

# Build the RPM
rpmbuild -ba fedora-pm-gui-rpm.spec --define "_topdir $(pwd)/rpmbuild"

# Install the new RPM
sudo dnf install -y rpmbuild/RPMS/x86_64/fedora-pm-gui-1.0.0-1.*.x86_64.rpm
```

### Option 2: Using existing build script

If you have a build script, update it to use the corrected spec file:

```bash
./install-gui-rpm.sh
```

## Verification Steps

After installation, verify the application appears in the menu:

1. **Check desktop file installation:**
   ```bash
   ls -l /usr/share/applications/fedora-pm.desktop
   ```

2. **Verify binary is installed:**
   ```bash
   which fedora-pm-gui
   ```

3. **Check desktop database:**
   ```bash
   update-desktop-database /usr/share/applications
   ```

4. **Look in application menu:**
   - Open your application launcher (Activities/Menu)
   - Search for "Fedora Package Manager"
   - The application should appear under System Tools or Package Management

5. **Test launching from command line:**
   ```bash
   fedora-pm-gui
   ```

## What the Post-Install Script Does

The `%post` section in the RPM spec now runs:
```bash
/usr/bin/update-desktop-database &> /dev/null || :
```

This command updates the desktop file cache, which is necessary for the application to appear in desktop environment menus immediately after installation.

## Troubleshooting

If the application still doesn't appear after installation:

1. **Manual desktop database update:**
   ```bash
   sudo update-desktop-database /usr/share/applications
   ```

2. **Check if desktop file is readable:**
   ```bash
   desktop-file-validate /usr/share/applications/fedora-pm.desktop
   ```

3. **Restart your desktop session** or log out and back in

4. **Check journal for errors:**
   ```bash
   journalctl -xe | grep fedora-pm
   ```

## Summary

The key issue was that the RPM spec file was trying to install non-existent Python scripts instead of building and installing the actual Rust binary. With the updated spec file, the RPM will now:

1. Build the `fedora-pm-gui` binary from Rust source
2. Install it to `/usr/bin/fedora-pm-gui`
3. Install the desktop file to `/usr/share/applications/`
4. Update the desktop database automatically
5. Make the application visible in your system's application menu