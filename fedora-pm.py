#!/usr/bin/env python3
"""
Fedora Package Manager - A modern package manager for Fedora Linux
"""

import subprocess
import sys
import json
import os
import argparse
from pathlib import Path
from typing import List, Optional, Dict
import shutil


class FedoraPackageManager:
    """Main package manager class"""
    
    def __init__(self, config_dir: Optional[str] = None):
        self.config_dir = Path(config_dir or os.path.expanduser("~/.fedora-pm"))
        self.config_file = self.config_dir / "config.json"
        self.cache_dir = self.config_dir / "cache"
        self.config_dir.mkdir(parents=True, exist_ok=True)
        self.cache_dir.mkdir(parents=True, exist_ok=True)
        self.config = self._load_config()
        self._check_dependencies()
    
    def _check_dependencies(self):
        """Check if required system tools are available"""
        required_tools = ['dnf', 'rpm']
        missing = []
        for tool in required_tools:
            if not shutil.which(tool):
                missing.append(tool)
        if missing:
            print(f"Error: Missing required tools: {', '.join(missing)}")
            print("Please install dnf and rpm packages.")
            sys.exit(1)
    
    def _load_config(self) -> Dict:
        """Load configuration from file"""
        default_config = {
            "auto_clean": False,
            "parallel_downloads": True,
            "fastest_mirror": True,
            "color_output": True,
            "history_file": str(self.config_dir / "history.json")
        }
        
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    user_config = json.load(f)
                    default_config.update(user_config)
            except json.JSONDecodeError:
                print(f"Warning: Invalid config file, using defaults")
        
        return default_config
    
    def _save_config(self):
        """Save configuration to file"""
        with open(self.config_file, 'w') as f:
            json.dump(self.config, f, indent=2)
    
    def _run_command(self, cmd: List[str], check: bool = True, capture: bool = False) -> Optional[str]:
        """Run a shell command"""
        try:
            result = subprocess.run(
                cmd,
                check=check,
                capture_output=capture,
                text=True
            )
            return result.stdout if capture else None
        except subprocess.CalledProcessError as e:
            if check:
                print(f"Error running command: {' '.join(cmd)}")
                print(e.stderr if hasattr(e, 'stderr') else str(e))
                sys.exit(1)
            return None
    
    def _log_action(self, action: str, packages: List[str]):
        """Log package management actions"""
        history_file = Path(self.config['history_file'])
        history = []
        
        if history_file.exists():
            try:
                with open(history_file, 'r') as f:
                    history = json.load(f)
            except json.JSONDecodeError:
                pass
        
        history.append({
            "action": action,
            "packages": packages,
            "timestamp": subprocess.run(['date', '+%Y-%m-%d %H:%M:%S'], 
                                       capture_output=True, text=True).stdout.strip()
        })
        
        with open(history_file, 'w') as f:
            json.dump(history, f, indent=2)
    
    def install(self, packages: List[str], yes: bool = False) -> bool:
        """Install packages"""
        if not packages:
            print("Error: No packages specified")
            return False
        
        print(f"Installing packages: {', '.join(packages)}")
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("install", packages)
            print(f"✓ Successfully installed: {', '.join(packages)}")
            return True
        except SystemExit:
            return False
    
    def remove(self, packages: List[str], yes: bool = False) -> bool:
        """Remove packages"""
        if not packages:
            print("Error: No packages specified")
            return False
        
        print(f"Removing packages: {', '.join(packages)}")
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("remove", packages)
            print(f"✓ Successfully removed: {', '.join(packages)}")
            return True
        except SystemExit:
            return False
    
    def update(self, packages: Optional[List[str]] = None, yes: bool = False) -> bool:
        """Update packages or system"""
        if packages:
            print(f"Updating packages: {', '.join(packages)}")
            cmd = ['sudo', 'dnf', 'update', '-y' if yes else ''] + packages
        else:
            print("Updating system...")
            cmd = ['sudo', 'dnf', 'update', '-y' if yes else '']
        
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("update", packages or ["system"])
            print("✓ System updated successfully")
            return True
        except SystemExit:
            return False
    
    def search(self, query: str) -> List[Dict]:
        """Search for packages"""
        print(f"Searching for: {query}")
        
        result = self._run_command(
            ['dnf', 'search', query],
            check=False,
            capture=True
        )
        
        if not result:
            print("No packages found")
            return []
        
        # Parse dnf search output
        packages = []
        current_package = None
        
        for line in result.split('\n'):
            line = line.strip()
            if not line:
                continue
            
            # DNF search format: "package.name : Description"
            if ' : ' in line:
                parts = line.split(' : ', 1)
                if len(parts) == 2:
                    packages.append({
                        "name": parts[0].strip(),
                        "description": parts[1].strip()
                    })
        
        return packages
    
    def info(self, package: str) -> Optional[Dict]:
        """Get package information"""
        result = self._run_command(
            ['rpm', '-qi', package],
            check=False,
            capture=True
        )
        
        if not result:
            # Try to get info from dnf if package not installed
            result = self._run_command(
                ['dnf', 'info', package],
                check=False,
                capture=True
            )
        
        if result:
            print(result)
            return {"package": package, "info": result}
        else:
            print(f"Package '{package}' not found")
            return None
    
    def list_installed(self, pattern: Optional[str] = None) -> List[str]:
        """List installed packages"""
        cmd = ['rpm', '-qa']
        if pattern:
            cmd.append(pattern)
        
        result = self._run_command(cmd, check=False, capture=True)
        
        if result:
            packages = [p.strip() for p in result.split('\n') if p.strip()]
            return packages
        return []
    
    def list_available(self, pattern: Optional[str] = None) -> List[str]:
        """List available packages"""
        cmd = ['dnf', 'list', 'available']
        if pattern:
            cmd.append(pattern)
        
        result = self._run_command(cmd, check=False, capture=True)
        
        if result:
            packages = []
            for line in result.split('\n')[1:]:  # Skip header
                if line.strip():
                    package_name = line.split()[0]
                    packages.append(package_name)
            return packages
        return []
    
    def clean(self, cache: bool = True, metadata: bool = True) -> bool:
        """Clean package cache and metadata"""
        print("Cleaning...")
        
        if cache:
            self._run_command(['sudo', 'dnf', 'clean', 'packages'], check=False)
        
        if metadata:
            self._run_command(['sudo', 'dnf', 'clean', 'metadata'], check=False)
        
        if cache and metadata:
            self._run_command(['sudo', 'dnf', 'clean', 'all'], check=False)
        
        print("✓ Clean completed")
        return True
    
    def history(self, limit: int = 10) -> List[Dict]:
        """Show package management history"""
        history_file = Path(self.config['history_file'])
        
        if not history_file.exists():
            print("No history found")
            return []
        
        try:
            with open(history_file, 'r') as f:
                history = json.load(f)
            
            # Show recent entries
            recent = history[-limit:] if len(history) > limit else history
            
            print(f"\nRecent package management history (last {len(recent)} entries):\n")
            for entry in reversed(recent):
                print(f"  [{entry['timestamp']}] {entry['action']}: {', '.join(entry['packages'])}")
            
            return recent
        except json.JSONDecodeError:
            print("Error reading history file")
            return []
    
    # Kernel management methods
    def kernel_current(self) -> Optional[str]:
        """Get current running kernel version"""
        result = self._run_command(['uname', '-r'], check=False, capture=True)
        if result:
            kernel = result.strip()
            print(f"Current kernel: {kernel}")
            return kernel
        return None
    
    def kernel_list_installed(self) -> List[Dict]:
        """List all installed kernels"""
        result = self._run_command(['rpm', '-qa', 'kernel*'], check=False, capture=True)
        
        if not result:
            print("No kernels found")
            return []
        
        kernels = []
        current_kernel = self._run_command(['uname', '-r'], check=False, capture=True)
        current_kernel = current_kernel.strip() if current_kernel else None
        
        for line in result.split('\n'):
            line = line.strip()
            if not line or not line.startswith('kernel'):
                continue
            
            # Check if it's a CachyOS kernel
            is_cachyos = 'cachyos' in line.lower()
            kernel_type = 'cachyos' if is_cachyos else 'standard'
            
            # Extract kernel version from package name
            # Format: kernel-5.19.0-1.fc37.x86_64 or kernel-cachyos-6.5.0-1.cachyos.x86_64
            kernel_parts = line.split('-')
            if len(kernel_parts) >= 3:
                # Try to extract version (e.g., 5.19.0-1.fc37)
                version_parts = kernel_parts[1:]
                version = '-'.join(version_parts).split('.')[0:3]  # Get major.minor.patch
                version_str = '.'.join(version_parts[0].split('.')[:3])
                
                # Check if this is the current kernel
                is_current = current_kernel and version_str in current_kernel
                
                kernels.append({
                    "package": line,
                    "version": version_str,
                    "full_version": '-'.join(version_parts),
                    "is_current": is_current,
                    "type": kernel_type
                })
        
        # Remove duplicates based on version
        seen_versions = set()
        unique_kernels = []
        for kernel in kernels:
            if kernel['version'] not in seen_versions:
                seen_versions.add(kernel['version'])
                unique_kernels.append(kernel)
        
        return unique_kernels
    
    def kernel_list_available(self) -> List[str]:
        """List available kernels from repositories"""
        result = self._run_command(
            ['dnf', 'list', 'available', 'kernel*'],
            check=False,
            capture=True
        )
        
        if not result:
            print("No available kernels found")
            return []
        
        kernels = []
        for line in result.split('\n')[1:]:  # Skip header
            if line.strip() and 'kernel' in line:
                package_name = line.split()[0]
                if package_name.startswith('kernel-') and 'kernel-core' not in package_name:
                    kernels.append(package_name)
        
        return kernels
    
    def kernel_install(self, version: Optional[str] = None, yes: bool = False) -> bool:
        """Install a kernel (latest or specific version)"""
        if version:
            print(f"Installing kernel version: {version}")
            packages = [f"kernel-{version}"]
        else:
            print("Installing latest kernel...")
            packages = ["kernel"]
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("kernel_install", packages)
            print(f"✓ Successfully installed kernel: {packages[0]}")
            print("  Note: Reboot to use the new kernel")
            return True
        except SystemExit:
            return False
    
    def kernel_remove(self, versions: List[str], yes: bool = False, keep_current: bool = True) -> bool:
        """Remove old kernels"""
        if not versions:
            print("Error: No kernel versions specified")
            return False
        
        current_kernel = self._run_command(['uname', '-r'], check=False, capture=True)
        current_kernel = current_kernel.strip() if current_kernel else None
        
        packages_to_remove = []
        for version in versions:
            if keep_current and current_kernel and version in current_kernel:
                print(f"Warning: Skipping current kernel {version}")
                continue
            
            # Find all kernel packages for this version
            result = self._run_command(
                ['rpm', '-qa', f'kernel*{version}*'],
                check=False,
                capture=True
            )
            
            if result:
                for pkg in result.split('\n'):
                    pkg = pkg.strip()
                    if pkg and pkg not in packages_to_remove:
                        packages_to_remove.append(pkg)
        
        if not packages_to_remove:
            print("No matching kernel packages found to remove")
            return False
        
        print(f"Removing kernel packages: {', '.join(packages_to_remove)}")
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages_to_remove
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("kernel_remove", packages_to_remove)
            print(f"✓ Successfully removed kernels")
            return True
        except SystemExit:
            return False
    
    def kernel_remove_old(self, keep: int = 2, yes: bool = False) -> bool:
        """Remove old kernels, keeping the specified number of newest ones"""
        kernels = self.kernel_list_installed()
        
        if len(kernels) <= keep:
            print(f"Only {len(kernels)} kernel(s) installed, keeping all")
            return True
        
        # Sort by version (simplified - in production you'd want proper version comparison)
        # For now, we'll just remove all except current and keep-1 others
        current_kernel = self._run_command(['uname', '-r'], check=False, capture=True)
        current_kernel = current_kernel.strip() if current_kernel else None
        
        # Filter out current kernel
        old_kernels = [k for k in kernels if not k.get('is_current', False)]
        
        if len(old_kernels) <= (keep - 1):
            print("No old kernels to remove")
            return True
        
        # Remove oldest kernels (keep only the newest keep-1)
        kernels_to_remove = old_kernels[:-(keep-1)] if len(old_kernels) > (keep-1) else []
        
        if not kernels_to_remove:
            print("No old kernels to remove")
            return True
        
        versions_to_remove = [k['version'] for k in kernels_to_remove]
        return self.kernel_remove(versions_to_remove, yes=yes, keep_current=True)
    
    def kernel_info(self, version: Optional[str] = None) -> Optional[Dict]:
        """Get information about a kernel"""
        if not version:
            version = self._run_command(['uname', '-r'], check=False, capture=True)
            if version:
                version = version.strip()
        
        if not version:
            print("Error: Could not determine kernel version")
            return None
        
        # Try to find kernel package
        result = self._run_command(
            ['rpm', '-qa', f'kernel*{version}*'],
            check=False,
            capture=True
        )
        
        if result:
            packages = [p.strip() for p in result.split('\n') if p.strip()]
            print(f"\nKernel version: {version}")
            print(f"Packages:\n")
            for pkg in packages:
                print(f"  {pkg}")
                
                # Get detailed info
                info = self._run_command(['rpm', '-qi', pkg], check=False, capture=True)
                if info:
                    # Extract key info
                    for line in info.split('\n'):
                        if any(key in line for key in ['Name', 'Version', 'Release', 'Size', 'Install Date']):
                            print(f"    {line}")
            
            return {"version": version, "packages": packages}
        else:
            print(f"Kernel version '{version}' not found")
            return None
    
    # CachyOS kernel management methods
    def cachyos_check_repo(self) -> Dict[str, bool]:
        """Check if CachyOS COPR repositories are enabled"""
        repos = {
            'gcc': False,
            'lto': False
        }
        
        result = self._run_command(['dnf', 'repolist', 'enabled'], check=False, capture=True)
        if result:
            if 'bieszczaders/kernel-cachyos' in result or 'kernel-cachyos' in result.lower():
                repos['gcc'] = True
            if 'bieszczaders/kernel-cachyos-lto' in result or 'kernel-cachyos-lto' in result.lower():
                repos['lto'] = True
        
        return repos
    
    def cachyos_enable_repo(self, repo_type: str = 'gcc', yes: bool = False) -> bool:
        """Enable CachyOS COPR repository"""
        if repo_type == 'gcc':
            repo = 'bieszczaders/kernel-cachyos'
            print("Enabling CachyOS kernel repository (GCC)...")
        elif repo_type == 'lto':
            repo = 'bieszczaders/kernel-cachyos-lto'
            print("Enabling CachyOS kernel repository (LLVM-ThinLTO)...")
        else:
            print(f"Error: Unknown repository type: {repo_type}")
            return False
        
        cmd = ['sudo', 'dnf', 'copr', 'enable', '-y' if yes else ''] + [repo]
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            print(f"✓ Successfully enabled CachyOS repository: {repo}")
            return True
        except SystemExit:
            return False
    
    def cachyos_list_available(self) -> List[Dict]:
        """List available CachyOS kernels"""
        repos = self.cachyos_check_repo()
        
        if not repos['gcc'] and not repos['lto']:
            print("CachyOS repositories are not enabled.")
            print("Enable them with: fedora-pm kernel cachyos enable")
            return []
        
        kernels = []
        
        # Check GCC repository
        if repos['gcc']:
            result = self._run_command(
                ['dnf', 'list', 'available', 'kernel-cachyos*'],
                check=False,
                capture=True
            )
            if result:
                for line in result.split('\n')[1:]:  # Skip header
                    if line.strip() and 'kernel-cachyos' in line:
                        package_name = line.split()[0]
                        if 'devel' not in package_name and 'headers' not in package_name:
                            kernel_type = self._get_cachyos_kernel_type(package_name)
                            kernels.append({
                                "package": package_name,
                                "type": kernel_type,
                                "build": "gcc"
                            })
        
        # Check LTO repository
        if repos['lto']:
            result = self._run_command(
                ['dnf', 'list', 'available', 'kernel-cachyos-lto*'],
                check=False,
                capture=True
            )
            if result:
                for line in result.split('\n')[1:]:  # Skip header
                    if line.strip() and 'kernel-cachyos-lto' in line:
                        package_name = line.split()[0]
                        if 'devel' not in package_name and 'headers' not in package_name:
                            kernel_type = self._get_cachyos_kernel_type(package_name)
                            kernels.append({
                                "package": package_name,
                                "type": kernel_type,
                                "build": "lto"
                            })
        
        return kernels
    
    def _get_cachyos_kernel_type(self, package_name: str) -> str:
        """Determine CachyOS kernel type from package name"""
        if 'lts' in package_name:
            return 'lts'
        elif 'rt' in package_name or 'realtime' in package_name:
            return 'rt'
        elif 'server' in package_name:
            return 'server'
        elif 'lto' in package_name:
            return 'lto'
        else:
            return 'default'
    
    def cachyos_install(self, kernel_type: str = 'default', build: str = 'gcc', yes: bool = False) -> bool:
        """Install a CachyOS kernel"""
        # Check/enable repository
        repos = self.cachyos_check_repo()
        target_repo = build.lower()
        
        if target_repo == 'gcc' and not repos['gcc']:
            print("CachyOS GCC repository not enabled. Enabling...")
            if not self.cachyos_enable_repo('gcc', yes=yes):
                return False
        elif target_repo == 'lto' and not repos['lto']:
            print("CachyOS LTO repository not enabled. Enabling...")
            if not self.cachyos_enable_repo('lto', yes=yes):
                return False
        
        # Determine package name
        if kernel_type == 'lts':
            if build == 'lto':
                package = 'kernel-cachyos-lts-lto'
            else:
                package = 'kernel-cachyos-lts'
        elif kernel_type == 'rt':
            if build == 'lto':
                package = 'kernel-cachyos-rt-lto'
            else:
                package = 'kernel-cachyos-rt'
        elif kernel_type == 'server':
            if build == 'lto':
                package = 'kernel-cachyos-server-lto'
            else:
                package = 'kernel-cachyos-server'
        else:  # default
            if build == 'lto':
                package = 'kernel-cachyos-lto'
            else:
                package = 'kernel-cachyos'
        
        # Also install devel-matched for kernel modules
        packages = [package, f"{package}-devel-matched"]
        
        print(f"Installing CachyOS kernel: {package} ({build.upper()} build)")
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("cachyos_kernel_install", packages)
            print(f"✓ Successfully installed CachyOS kernel: {package}")
            print("  Note: Reboot to use the new kernel")
            return True
        except SystemExit:
            return False
    
    def cachyos_check_cpu_support(self) -> Dict[str, bool]:
        """Check CPU instruction set support for CachyOS kernels"""
        support = {
            'x86-64-v2': False,
            'x86-64-v3': False,
            'x86-64-v4': False
        }
        
        # Try to check CPU support using ld-linux
        ld_paths = [
            '/lib64/ld-linux-x86-64.so.2',
            '/usr/lib64/ld-linux-x86-64.so.2',
            '/lib/ld-linux-x86-64.so.2'
        ]
        
        result = None
        for ld_path in ld_paths:
            if os.path.exists(ld_path):
                result = self._run_command([ld_path, '--help'], check=False, capture=True)
                if result:
                    break
        
        if result:
            # Look for supported instruction sets in the output
            for line in result.split('\n'):
                line_lower = line.lower()
                if 'x86-64-v2' in line_lower or 'x86_64_v2' in line_lower:
                    support['x86-64-v2'] = True
                if 'x86-64-v3' in line_lower or 'x86_64_v3' in line_lower:
                    support['x86-64-v3'] = True
                if 'x86-64-v4' in line_lower or 'x86_64_v4' in line_lower:
                    support['x86-64-v4'] = True
        
        # Also try checking via /proc/cpuinfo as fallback
        if not any(support.values()):
            try:
                with open('/proc/cpuinfo', 'r') as f:
                    cpuinfo = f.read()
                    # Basic check - if we have modern CPU features, likely v3+
                    if 'avx' in cpuinfo.lower() and 'avx2' in cpuinfo.lower():
                        support['x86-64-v3'] = True
                        support['x86-64-v2'] = True
            except:
                pass
        
        return support
    
    # Driver management methods
    def driver_detect_gpu(self) -> Dict[str, str]:
        """Detect installed GPU hardware"""
        gpus = {}
        
        # Check for Nvidia GPU
        result = self._run_command(['lspci'], check=False, capture=True)
        if result and 'NVIDIA' in result.upper():
            nvidia_lines = [line for line in result.split('\n') if 'NVIDIA' in line.upper() or 'nvidia' in line.lower()]
            if nvidia_lines:
                gpus['nvidia'] = nvidia_lines[0].strip()
        
        # Check for AMD GPU
        if result and ('AMD' in result.upper() or 'RADEON' in result.upper()):
            amd_lines = [line for line in result.split('\n') if 'AMD' in line.upper() or 'RADEON' in result.upper()]
            if amd_lines:
                gpus['amd'] = amd_lines[0].strip()
        
        # Check for Intel GPU
        if result and 'INTEL' in result.upper():
            intel_lines = [line for line in result.split('\n') if 'INTEL' in line.upper()]
            if intel_lines:
                gpus['intel'] = intel_lines[0].strip()
        
        return gpus
    
    def driver_check_nvidia(self) -> Optional[Dict]:
        """Check Nvidia driver status"""
        info = {}
        
        # Check if nvidia-smi is available
        nvidia_smi = shutil.which('nvidia-smi')
        if nvidia_smi:
            result = self._run_command(['nvidia-smi'], check=False, capture=True)
            if result:
                info['installed'] = True
                info['nvidia-smi'] = result
                
                # Try to extract driver version
                for line in result.split('\n'):
                    if 'Driver Version:' in line:
                        version = line.split('Driver Version:')[1].strip().split()[0]
                        info['version'] = version
                        break
            else:
                info['installed'] = False
        else:
            info['installed'] = False
        
        # Check installed packages
        result = self._run_command(['rpm', '-qa', 'nvidia*'], check=False, capture=True)
        if result:
            packages = [p.strip() for p in result.split('\n') if p.strip()]
            info['packages'] = packages
        else:
            info['packages'] = []
        
        return info
    
    def driver_list_nvidia_available(self) -> List[str]:
        """List available Nvidia drivers from repositories"""
        # Check if RPM Fusion is enabled
        result = self._run_command(['dnf', 'repolist'], check=False, capture=True)
        rpmfusion_enabled = result and ('rpmfusion' in result.lower() if result else False)
        
        if not rpmfusion_enabled:
            print("Note: RPM Fusion repositories may not be enabled.")
            print("      Nvidia drivers are typically available from RPM Fusion.")
        
        # Search for available Nvidia drivers
        result = self._run_command(
            ['dnf', 'list', 'available', 'akmod-nvidia*'],
            check=False,
            capture=True
        )
        
        drivers = []
        if result:
            for line in result.split('\n')[1:]:  # Skip header
                if line.strip() and 'akmod-nvidia' in line:
                    package_name = line.split()[0]
                    # Extract version if possible
                    if 'akmod-nvidia' in package_name:
                        drivers.append(package_name)
        
        # Also check for regular nvidia drivers
        result = self._run_command(
            ['dnf', 'list', 'available', 'nvidia-driver*'],
            check=False,
            capture=True
        )
        
        if result:
            for line in result.split('\n')[1:]:
                if line.strip() and 'nvidia-driver' in line:
                    package_name = line.split()[0]
                    if package_name not in drivers:
                        drivers.append(package_name)
        
        return drivers
    
    def driver_install_nvidia(self, version: Optional[str] = None, cuda: bool = False, yes: bool = False) -> bool:
        """Install Nvidia drivers"""
        print("Installing Nvidia drivers...")
        
        # Check if RPM Fusion is enabled
        result = self._run_command(['dnf', 'repolist', 'enabled'], check=False, capture=True)
        rpmfusion_enabled = result and ('rpmfusion' in result.lower() if result else False)
        
        if not rpmfusion_enabled:
            print("\n⚠ Warning: RPM Fusion repositories may not be enabled.")
            print("  Nvidia drivers require RPM Fusion repositories.")
            print("  You may need to enable them first:")
            print("    sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm")
            print("    sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm")
            if not yes:
                response = input("\nContinue anyway? (y/N): ")
                if response.lower() != 'y':
                    print("Installation cancelled")
                    return False
        
        packages = []
        
        if version:
            # Try to install specific version
            packages.append(f"akmod-nvidia-{version}")
        else:
            # Install latest akmod-nvidia (recommended for Fedora)
            packages.append("akmod-nvidia")
        
        # Add xorg-x11-drv-nvidia-cuda if CUDA is requested
        if cuda:
            packages.append("xorg-x11-drv-nvidia-cuda")
            packages.append("xorg-x11-drv-nvidia-cuda-libs")
        
        # Also install xorg-x11-drv-nvidia for X11 support
        if 'xorg-x11-drv-nvidia' not in ' '.join(packages):
            packages.append("xorg-x11-drv-nvidia")
        
        print(f"Installing packages: {', '.join(packages)}")
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("driver_install_nvidia", packages)
            print("\n✓ Nvidia drivers installed successfully")
            print("\n⚠ Important next steps:")
            print("  1. Rebuild kernel modules: sudo akmods --force")
            print("  2. Reboot your system")
            print("  3. After reboot, verify with: nvidia-smi")
            if cuda:
                print("  4. CUDA support has been installed")
            return True
        except SystemExit:
            return False
    
    def driver_remove_nvidia(self, yes: bool = False) -> bool:
        """Remove Nvidia drivers"""
        print("Removing Nvidia drivers...")
        
        # Find all Nvidia packages
        result = self._run_command(['rpm', '-qa', 'nvidia*'], check=False, capture=True)
        if not result:
            print("No Nvidia drivers found to remove")
            return True
        
        packages = [p.strip() for p in result.split('\n') if p.strip()]
        
        # Also check for akmod-nvidia
        result = self._run_command(['rpm', '-qa', 'akmod-nvidia*'], check=False, capture=True)
        if result:
            akmod_packages = [p.strip() for p in result.split('\n') if p.strip()]
            packages.extend(akmod_packages)
        
        if not packages:
            print("No Nvidia drivers found to remove")
            return True
        
        print(f"Removing packages: {', '.join(packages)}")
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("driver_remove_nvidia", packages)
            print("✓ Nvidia drivers removed successfully")
            print("  Note: You may need to reboot for changes to take effect")
            return True
        except SystemExit:
            return False
    
    def driver_status(self) -> Dict:
        """Get status of all drivers"""
        status = {}
        
        # Detect GPUs
        gpus = self.driver_detect_gpu()
        status['gpus'] = gpus
        
        # Check Nvidia
        if 'nvidia' in gpus:
            nvidia_status = self.driver_check_nvidia()
            status['nvidia'] = nvidia_status
        
        # Check for other drivers
        # AMD
        if 'amd' in gpus:
            result = self._run_command(['rpm', '-qa', 'mesa*'], check=False, capture=True)
            status['amd'] = {
                'detected': True,
                'packages': [p.strip() for p in result.split('\n') if p.strip()] if result else []
            }
        
        # Intel
        if 'intel' in gpus:
            result = self._run_command(['rpm', '-qa', 'mesa*'], check=False, capture=True)
            status['intel'] = {
                'detected': True,
                'packages': [p.strip() for p in result.split('\n') if p.strip()] if result else []
            }
        
        return status
    
    def driver_install_cuda(self, yes: bool = False) -> bool:
        """Install CUDA toolkit for Nvidia"""
        print("Installing CUDA toolkit...")
        
        # Check if Nvidia drivers are installed
        nvidia_status = self.driver_check_nvidia()
        if not nvidia_status.get('installed', False):
            print("⚠ Warning: Nvidia drivers do not appear to be installed.")
            print("  It's recommended to install Nvidia drivers first.")
            if not yes:
                response = input("Continue anyway? (y/N): ")
                if response.lower() != 'y':
                    return False
        
        packages = [
            "cuda",
            "cuda-toolkit",
            "nvidia-cuda-toolkit"
        ]
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("driver_install_cuda", packages)
            print("✓ CUDA toolkit installed successfully")
            print("  Note: You may need to set up environment variables:")
            print("    export PATH=/usr/local/cuda/bin:$PATH")
            print("    export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH")
            return True
        except SystemExit:
            return False


def main():
    """Main CLI entry point"""
    parser = argparse.ArgumentParser(
        description='Fedora Package Manager - A modern package manager for Fedora Linux',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  fedora-pm install vim git
  fedora-pm remove old-package
  fedora-pm update
  fedora-pm search python
  fedora-pm info vim
  fedora-pm list installed
  fedora-pm clean
  fedora-pm kernel list
  fedora-pm kernel install
  fedora-pm kernel remove-old
  fedora-pm kernel cachyos list
  fedora-pm kernel cachyos install
  fedora-pm kernel cachyos enable gcc
  fedora-pm driver status
  fedora-pm driver install nvidia
  fedora-pm driver install nvidia --cuda
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Commands')
    
    # Install command
    install_parser = subparsers.add_parser('install', help='Install packages')
    install_parser.add_argument('packages', nargs='+', help='Package names to install')
    install_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Remove command
    remove_parser = subparsers.add_parser('remove', help='Remove packages')
    remove_parser.add_argument('packages', nargs='+', help='Package names to remove')
    remove_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Update command
    update_parser = subparsers.add_parser('update', help='Update packages or system')
    update_parser.add_argument('packages', nargs='*', help='Specific packages to update (optional)')
    update_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Search command
    search_parser = subparsers.add_parser('search', help='Search for packages')
    search_parser.add_argument('query', help='Search query')
    
    # Info command
    info_parser = subparsers.add_parser('info', help='Show package information')
    info_parser.add_argument('package', help='Package name')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List packages')
    list_parser.add_argument('type', choices=['installed', 'available'], help='List type')
    list_parser.add_argument('pattern', nargs='?', help='Filter pattern (optional)')
    
    # Clean command
    clean_parser = subparsers.add_parser('clean', help='Clean cache and metadata')
    clean_parser.add_argument('--no-cache', action='store_true', help='Skip cache cleaning')
    clean_parser.add_argument('--no-metadata', action='store_true', help='Skip metadata cleaning')
    
    # History command
    history_parser = subparsers.add_parser('history', help='Show package management history')
    history_parser.add_argument('-n', '--limit', type=int, default=10, help='Number of entries to show')
    
    # Kernel command
    kernel_parser = subparsers.add_parser('kernel', help='Kernel management')
    kernel_subparsers = kernel_parser.add_subparsers(dest='kernel_command', help='Kernel commands')
    
    # Kernel list
    kernel_list_parser = kernel_subparsers.add_parser('list', help='List installed kernels')
    kernel_list_parser.add_argument('--available', action='store_true', help='List available kernels from repositories')
    
    # Kernel current
    kernel_subparsers.add_parser('current', help='Show current running kernel')
    
    # Kernel install
    kernel_install_parser = kernel_subparsers.add_parser('install', help='Install a kernel')
    kernel_install_parser.add_argument('version', nargs='?', help='Kernel version to install (default: latest)')
    kernel_install_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Kernel remove
    kernel_remove_parser = kernel_subparsers.add_parser('remove', help='Remove kernel(s)')
    kernel_remove_parser.add_argument('versions', nargs='+', help='Kernel versions to remove')
    kernel_remove_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    kernel_remove_parser.add_argument('--force', action='store_true', help='Allow removing current kernel (dangerous)')
    
    # Kernel remove old
    kernel_remove_old_parser = kernel_subparsers.add_parser('remove-old', help='Remove old kernels')
    kernel_remove_old_parser.add_argument('--keep', type=int, default=2, help='Number of kernels to keep (default: 2)')
    kernel_remove_old_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Kernel info
    kernel_info_parser = kernel_subparsers.add_parser('info', help='Show kernel information')
    kernel_info_parser.add_argument('version', nargs='?', help='Kernel version (default: current)')
    
    # Kernel CachyOS subcommands
    kernel_cachyos_parser = kernel_subparsers.add_parser('cachyos', help='CachyOS kernel management')
    kernel_cachyos_subparsers = kernel_cachyos_parser.add_subparsers(dest='cachyos_command', help='CachyOS commands')
    
    # CachyOS list
    kernel_cachyos_subparsers.add_parser('list', help='List available CachyOS kernels')
    
    # CachyOS enable repo
    kernel_cachyos_enable_parser = kernel_cachyos_subparsers.add_parser('enable', help='Enable CachyOS repository')
    kernel_cachyos_enable_parser.add_argument('type', choices=['gcc', 'lto', 'both'], default='gcc', nargs='?', help='Repository type (default: gcc)')
    kernel_cachyos_enable_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # CachyOS check repo
    kernel_cachyos_subparsers.add_parser('check', help='Check CachyOS repository status')
    
    # CachyOS install
    kernel_cachyos_install_parser = kernel_cachyos_subparsers.add_parser('install', help='Install CachyOS kernel')
    kernel_cachyos_install_parser.add_argument('type', choices=['default', 'lts', 'rt', 'server'], default='default', nargs='?', help='Kernel type (default: default)')
    kernel_cachyos_install_parser.add_argument('--build', choices=['gcc', 'lto'], default='gcc', help='Build type (default: gcc)')
    kernel_cachyos_install_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # CachyOS check CPU
    kernel_cachyos_subparsers.add_parser('check-cpu', help='Check CPU instruction set support')
    
    # Driver command
    driver_parser = subparsers.add_parser('driver', help='Driver management')
    driver_subparsers = driver_parser.add_subparsers(dest='driver_command', help='Driver commands')
    
    # Driver status
    driver_subparsers.add_parser('status', help='Show driver status for all detected GPUs')
    
    # Driver detect
    driver_subparsers.add_parser('detect', help='Detect installed GPU hardware')
    
    # Driver install nvidia
    driver_install_nvidia_parser = driver_subparsers.add_parser('install', help='Install drivers')
    driver_install_nvidia_parser.add_argument('type', choices=['nvidia'], help='Driver type to install')
    driver_install_nvidia_parser.add_argument('--version', help='Specific driver version (optional)')
    driver_install_nvidia_parser.add_argument('--cuda', action='store_true', help='Also install CUDA support')
    driver_install_nvidia_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Driver remove nvidia
    driver_remove_nvidia_parser = driver_subparsers.add_parser('remove', help='Remove drivers')
    driver_remove_nvidia_parser.add_argument('type', choices=['nvidia'], help='Driver type to remove')
    driver_remove_nvidia_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    # Driver check nvidia
    driver_subparsers.add_parser('check', help='Check Nvidia driver status')
    
    # Driver list nvidia
    driver_list_parser = driver_subparsers.add_parser('list', help='List available drivers')
    driver_list_parser.add_argument('type', choices=['nvidia'], help='Driver type to list')
    
    # Driver install cuda
    driver_cuda_parser = driver_subparsers.add_parser('cuda', help='Install CUDA toolkit')
    driver_cuda_parser.add_argument('action', choices=['install'], help='Action to perform')
    driver_cuda_parser.add_argument('-y', '--yes', action='store_true', help='Auto-confirm')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    pm = FedoraPackageManager()
    
    if args.command == 'install':
        pm.install(args.packages, yes=args.yes)
    elif args.command == 'remove':
        pm.remove(args.packages, yes=args.yes)
    elif args.command == 'update':
        pm.update(args.packages if args.packages else None, yes=args.yes)
    elif args.command == 'search':
        packages = pm.search(args.query)
        if packages:
            print(f"\nFound {len(packages)} packages:\n")
            for pkg in packages[:20]:  # Limit output
                print(f"  {pkg['name']}")
                print(f"    {pkg['description']}\n")
    elif args.command == 'info':
        pm.info(args.package)
    elif args.command == 'list':
        if args.type == 'installed':
            packages = pm.list_installed(args.pattern)
            print(f"\nInstalled packages ({len(packages)}):\n")
            for pkg in packages[:50]:  # Limit output
                print(f"  {pkg}")
        else:
            packages = pm.list_available(args.pattern)
            print(f"\nAvailable packages ({len(packages)}):\n")
            for pkg in packages[:50]:  # Limit output
                print(f"  {pkg}")
    elif args.command == 'clean':
        pm.clean(cache=not args.no_cache, metadata=not args.no_metadata)
    elif args.command == 'history':
        pm.history(limit=args.limit)
    elif args.command == 'kernel':
        if not args.kernel_command:
            kernel_parser.print_help()
            sys.exit(1)
        
        if args.kernel_command == 'current':
            pm.kernel_current()
        elif args.kernel_command == 'list':
            if args.available:
                kernels = pm.kernel_list_available()
                if kernels:
                    print(f"\nAvailable kernels ({len(kernels)}):\n")
                    for kernel in kernels[:20]:  # Limit output
                        print(f"  {kernel}")
            else:
                kernels = pm.kernel_list_installed()
                if kernels:
                    print(f"\nInstalled kernels ({len(kernels)}):\n")
                    for kernel in kernels:
                        current_marker = " (current)" if kernel.get('is_current') else ""
                        type_marker = f" [{kernel.get('type', 'standard').upper()}]" if kernel.get('type') == 'cachyos' else ""
                        print(f"  {kernel['version']}{current_marker}{type_marker}")
                        print(f"    Package: {kernel['package']}\n")
        elif args.kernel_command == 'install':
            pm.kernel_install(version=args.version, yes=args.yes)
        elif args.kernel_command == 'remove':
            pm.kernel_remove(args.versions, yes=args.yes, keep_current=not args.force)
        elif args.kernel_command == 'remove-old':
            pm.kernel_remove_old(keep=args.keep, yes=args.yes)
        elif args.kernel_command == 'info':
            pm.kernel_info(version=args.version)
        elif args.kernel_command == 'cachyos':
            if not args.cachyos_command:
                kernel_cachyos_parser.print_help()
                sys.exit(1)
            
            if args.cachyos_command == 'list':
                kernels = pm.cachyos_list_available()
                if kernels:
                    print(f"\nAvailable CachyOS kernels ({len(kernels)}):\n")
                    for kernel in kernels:
                        build_info = f" ({kernel['build'].upper()} build)"
                        print(f"  {kernel['package']}{build_info}")
                        print(f"    Type: {kernel['type']}\n")
                else:
                    print("No CachyOS kernels found or repositories not enabled")
                    print("Enable repositories with: fedora-pm kernel cachyos enable")
            elif args.cachyos_command == 'enable':
                if args.type == 'both':
                    pm.cachyos_enable_repo('gcc', yes=args.yes)
                    pm.cachyos_enable_repo('lto', yes=args.yes)
                else:
                    pm.cachyos_enable_repo(args.type, yes=args.yes)
            elif args.cachyos_command == 'check':
                repos = pm.cachyos_check_repo()
                print("\n=== CachyOS Repository Status ===\n")
                print(f"GCC Repository: {'✓ Enabled' if repos['gcc'] else '✗ Not enabled'}")
                print(f"LTO Repository: {'✓ Enabled' if repos['lto'] else '✗ Not enabled'}")
                if not repos['gcc'] and not repos['lto']:
                    print("\nEnable repositories with:")
                    print("  fedora-pm kernel cachyos enable gcc")
                    print("  fedora-pm kernel cachyos enable lto")
            elif args.cachyos_command == 'install':
                pm.cachyos_install(kernel_type=args.type, build=args.build, yes=args.yes)
            elif args.cachyos_command == 'check-cpu':
                support = pm.cachyos_check_cpu_support()
                print("\n=== CPU Instruction Set Support ===\n")
                for isa, supported in support.items():
                    status = "✓ Supported" if supported else "✗ Not supported"
                    print(f"{isa}: {status}")
                
                print("\nCachyOS Kernel Requirements:")
                print("  - Most kernels require: x86-64-v3")
                print("  - LTS/Server kernels require: x86-64-v2")
                
                if not support.get('x86-64-v3', False) and not support.get('x86-64-v2', False):
                    print("\n⚠ Warning: Your CPU may not support CachyOS kernels")
    elif args.command == 'driver':
        if not args.driver_command:
            driver_parser.print_help()
            sys.exit(1)
        
        if args.driver_command == 'status':
            status = pm.driver_status()
            print("\n=== Driver Status ===\n")
            
            if 'gpus' in status and status['gpus']:
                print("Detected GPUs:")
                for gpu_type, gpu_info in status['gpus'].items():
                    print(f"  {gpu_type.upper()}: {gpu_info}")
                print()
            
            if 'nvidia' in status:
                nvidia = status['nvidia']
                print("Nvidia Driver:")
                if nvidia.get('installed'):
                    print(f"  Status: ✓ Installed")
                    if 'version' in nvidia:
                        print(f"  Version: {nvidia['version']}")
                    if 'packages' in nvidia and nvidia['packages']:
                        print(f"  Packages: {len(nvidia['packages'])} installed")
                        for pkg in nvidia['packages'][:5]:  # Show first 5
                            print(f"    - {pkg}")
                else:
                    print("  Status: ✗ Not installed")
                print()
            
            if 'amd' in status:
                amd = status['amd']
                print("AMD Driver:")
                print(f"  Status: {'✓ Detected' if amd.get('detected') else '✗ Not detected'}")
                if amd.get('packages'):
                    print(f"  Packages: {len(amd['packages'])} installed")
                print()
            
            if 'intel' in status:
                intel = status['intel']
                print("Intel Driver:")
                print(f"  Status: {'✓ Detected' if intel.get('detected') else '✗ Not detected'}")
                if intel.get('packages'):
                    print(f"  Packages: {len(intel['packages'])} installed")
                print()
        elif args.driver_command == 'detect':
            gpus = pm.driver_detect_gpu()
            if gpus:
                print("\nDetected GPUs:\n")
                for gpu_type, gpu_info in gpus.items():
                    print(f"  {gpu_type.upper()}: {gpu_info}")
            else:
                print("No GPUs detected")
        elif args.driver_command == 'install':
            if args.type == 'nvidia':
                pm.driver_install_nvidia(version=args.version, cuda=args.cuda, yes=args.yes)
        elif args.driver_command == 'remove':
            if args.type == 'nvidia':
                pm.driver_remove_nvidia(yes=args.yes)
        elif args.driver_command == 'check':
            nvidia_status = pm.driver_check_nvidia()
            if nvidia_status:
                print("\n=== Nvidia Driver Status ===\n")
                if nvidia_status.get('installed'):
                    print("Status: ✓ Installed")
                    if 'version' in nvidia_status:
                        print(f"Driver Version: {nvidia_status['version']}")
                    if 'packages' in nvidia_status and nvidia_status['packages']:
                        print(f"\nInstalled packages ({len(nvidia_status['packages'])}):")
                        for pkg in nvidia_status['packages']:
                            print(f"  - {pkg}")
                    if 'nvidia-smi' in nvidia_status:
                        print("\nNvidia-SMI Output:")
                        print(nvidia_status['nvidia-smi'])
                else:
                    print("Status: ✗ Not installed")
                    if 'packages' in nvidia_status and nvidia_status['packages']:
                        print(f"\nFound packages ({len(nvidia_status['packages'])}):")
                        for pkg in nvidia_status['packages']:
                            print(f"  - {pkg}")
        elif args.driver_command == 'list':
            if args.type == 'nvidia':
                drivers = pm.driver_list_nvidia_available()
                if drivers:
                    print(f"\nAvailable Nvidia drivers ({len(drivers)}):\n")
                    for driver in drivers[:20]:  # Limit output
                        print(f"  {driver}")
                else:
                    print("No available Nvidia drivers found")
                    print("Note: Make sure RPM Fusion repositories are enabled")
        elif args.driver_command == 'cuda':
            if args.action == 'install':
                pm.driver_install_cuda(yes=args.yes)


if __name__ == '__main__':
    main()

