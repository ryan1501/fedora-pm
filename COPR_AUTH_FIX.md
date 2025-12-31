# COPR Authentication and Upload Guide

## 🔧 Authentication Issues

The error `The 'username|@groupname' not specified` indicates authentication problems with COPR CLI.

## 🚀 Quick Solutions

### Option 1: Web Upload (Recommended)
```bash
# 1. Create source tarball
tar -czf fedora-pm-1.1.0.tar.gz \
  --exclude=vcs \
  --exclude='target*' \
  --exclude='rpmbuild*' \
  --exclude='.fingerprint*' \
  --exclude='*.tmp' \
  .

# 2. Upload via web interface
# Visit: https://copr.fedorainfracloud.org/coprs/uncodedchristiangamer/fedora-pm/new_build/
# Upload: fedora-pm-1.1.0.tar.gz
# Configure: mock build, multiple Fedora versions
```

### Option 2: API Token Upload
```bash
# 1. Generate API Token
# Visit: https://copr.fedorainfracloud.org/api/token/
# Click "Generate new token" and copy it

# 2. Set token
export COPR_TOKEN="your-generated-token"

# 3. Upload
copr upload --name fedora-pm --version 1.1.0 fedora-pm-1.1.0.tar.gz
```

### Option 3: Direct Username Upload
```bash
# Using username flag
copr --username uncodedchristiangamer upload --name fedora-pm --version 1.1.0 fedora-pm-1.1.0.tar.gz
```

## 📋 Step-by-Step Guide

### Method 1: Web Interface (Easiest)
1. **Create tarball**:
   ```bash
   tar -czf fedora-pm-1.1.0.tar.gz --exclude=target* --exclude=rpmbuild* .
   ```

2. **Visit COPR**:
   https://copr.fedorainfracloud.org/coprs/uncodedchristiangamer/fedora-pm/new_build/

3. **Upload and configure**:
   - Upload: `fedora-pm-1.1.0.tar.gz`
   - Build Method: `mock`
   - Fedora Versions: 39, 40, 41, 42, 43
   - Changelog: "GitHub-based self-update integration"

### Method 2: API Token Upload
1. **Generate token**:
   https://copr.fedorainfracloud.org/api/token/

2. **Upload with token**:
   ```bash
   export COPR_TOKEN="your-token"
   copr upload --name fedora-pm --version 1.1.0 fedora-pm-1.1.0.tar.gz
   ```

### Method 3: Username Parameter
```bash
copr --username uncodedchristiangamer upload --name fedora-pm --version 1.1.0 fedora-pm-1.1.0.tar.gz
```

## 🔍 Troubleshooting

### Authentication Issues
- **Generate new API token** if old one expired
- **Use web upload** if CLI authentication fails
- **Check COPR username** matches exactly

### Upload Issues
- **Verify tarball structure** - should have top-level fedora-pm-1.1.0/ directory
- **Check file permissions** - ensure readable by upload process
- **Monitor build logs** via COPR web interface

### Build Issues
- **Check spec file** - proper BuildRequires and dependencies
- **Exclude build artifacts** - target/, rpmbuild/, etc.
- **Test locally**: `fedpkg-packager --build fedora-pm.spec`

## 📖 Documentation

- **COPR API**: https://copr.fedorainfracloud.org/api/
- **copr-cli docs**: https://pythonhosted.org/copr-cli/
- **Fedora Packaging**: https://docs.fedoraproject.org/en-US/packaging/

## 🌐 URLs

- **COPR Main**: https://copr.fedorainfracloud.org/
- **Your COPR**: https://copr.fedorainfracloud.org/coprs/uncodedchristiangamer/fedora-pm/
- **API Tokens**: https://copr.fedorainfracloud.org/api/token/

Choose the method that works best for your setup! 🚀