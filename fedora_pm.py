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
            raise RuntimeError(f"Missing required tools: {', '.join(missing)}. Please install dnf and rpm packages.")
    
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
                pass
        
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
                raise RuntimeError(f"Error running command: {' '.join(cmd)}\n{e.stderr if hasattr(e, 'stderr') else str(e)}")
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
            raise ValueError("No packages specified")
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("install", packages)
            return True
        except RuntimeError:
            return False
    
    def remove(self, packages: List[str], yes: bool = False) -> bool:
        """Remove packages"""
        if not packages:
            raise ValueError("No packages specified")
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("remove", packages)
            return True
        except RuntimeError:
            return False
    
    def update(self, packages: Optional[List[str]] = None, yes: bool = False) -> bool:
        """Update packages or system"""
        if packages:
            cmd = ['sudo', 'dnf', 'update', '-y' if yes else ''] + packages
        else:
            cmd = ['sudo', 'dnf', 'update', '-y' if yes else '']
        
        cmd = [c for c in cmd if c]  # Remove empty strings
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("update", packages or ["system"])
            return True
        except RuntimeError:
            return False
    
    def search(self, query: str) -> List[Dict]:
        """Search for packages"""
        result = self._run_command(
            ['dnf', 'search', query],
            check=False,
            capture=True
        )
        
        if not result:
            return []
        
        # Parse dnf search output
        packages = []
        
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
            return {"package": package, "info": result}
        else:
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
        if cache:
            self._run_command(['sudo', 'dnf', 'clean', 'packages'], check=False)
        
        if metadata:
            self._run_command(['sudo', 'dnf', 'clean', 'metadata'], check=False)
        
        if cache and metadata:
            self._run_command(['sudo', 'dnf', 'clean', 'all'], check=False)
        
        return True
    
    def history(self, limit: int = 10) -> List[Dict]:
        """Show package management history"""
        history_file = Path(self.config['history_file'])
        
        if not history_file.exists():
            return []
        
        try:
            with open(history_file, 'r') as f:
                history = json.load(f)
            
            # Show recent entries
            recent = history[-limit:] if len(history) > limit else history
            return recent
        except json.JSONDecodeError:
            return []
    
    # Kernel management methods
    def kernel_current(self) -> Optional[str]:
        """Get current running kernel version"""
        result = self._run_command(['uname', '-r'], check=False, capture=True)
        if result:
            return result.strip()
        return None
    
    def kernel_list_installed(self) -> List[Dict]:
        """List all installed kernels"""
        result = self._run_command(['rpm', '-qa', 'kernel*'], check=False, capture=True)
        
        if not result:
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
            kernel_parts = line.split('-')
            if len(kernel_parts) >= 3:
                version_parts = kernel_parts[1:]
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
            packages = [f"kernel-{version}"]
        else:
            packages = ["kernel"]
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("kernel_install", packages)
            return True
        except RuntimeError:
            return False
    
    def kernel_remove(self, versions: List[str], yes: bool = False, keep_current: bool = True) -> bool:
        """Remove old kernels"""
        if not versions:
            raise ValueError("No kernel versions specified")
        
        current_kernel = self._run_command(['uname', '-r'], check=False, capture=True)
        current_kernel = current_kernel.strip() if current_kernel else None
        
        packages_to_remove = []
        for version in versions:
            if keep_current and current_kernel and version in current_kernel:
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
            return False
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages_to_remove
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("kernel_remove", packages_to_remove)
            return True
        except RuntimeError:
            return False
    
    def kernel_remove_old(self, keep: int = 2, yes: bool = False) -> bool:
        """Remove old kernels, keeping the specified number of newest ones"""
        kernels = self.kernel_list_installed()
        
        if len(kernels) <= keep:
            return True
        
        current_kernel = self._run_command(['uname', '-r'], check=False, capture=True)
        current_kernel = current_kernel.strip() if current_kernel else None
        
        # Filter out current kernel
        old_kernels = [k for k in kernels if not k.get('is_current', False)]
        
        if len(old_kernels) <= (keep - 1):
            return True
        
        # Remove oldest kernels (keep only the newest keep-1)
        kernels_to_remove = old_kernels[:-(keep-1)] if len(old_kernels) > (keep-1) else []
        
        if not kernels_to_remove:
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
            return None
        
        # Try to find kernel package
        result = self._run_command(
            ['rpm', '-qa', f'kernel*{version}*'],
            check=False,
            capture=True
        )
        
        if result:
            packages = [p.strip() for p in result.split('\n') if p.strip()]
            return {"version": version, "packages": packages}
        else:
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
        elif repo_type == 'lto':
            repo = 'bieszczaders/kernel-cachyos-lto'
        else:
            raise ValueError(f"Unknown repository type: {repo_type}")
        
        cmd = ['sudo', 'dnf', 'copr', 'enable', '-y' if yes else ''] + [repo]
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            return True
        except RuntimeError:
            return False
    
    def cachyos_list_available(self) -> List[Dict]:
        """List available CachyOS kernels"""
        repos = self.cachyos_check_repo()
        
        if not repos['gcc'] and not repos['lto']:
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
            if not self.cachyos_enable_repo('gcc', yes=yes):
                return False
        elif target_repo == 'lto' and not repos['lto']:
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
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("cachyos_kernel_install", packages)
            return True
        except RuntimeError:
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
            # Filter per line rather than the whole output to avoid false matches
            amd_lines = [
                line for line in result.split('\n')
                if 'AMD' in line.upper() or 'RADEON' in line.upper()
            ]
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
        
        cmd = ['sudo', 'dnf', 'install', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("driver_install_nvidia", packages)
            return True
        except RuntimeError:
            return False
    
    def driver_remove_nvidia(self, yes: bool = False) -> bool:
        """Remove Nvidia drivers"""
        # Find all Nvidia packages
        result = self._run_command(['rpm', '-qa', 'nvidia*'], check=False, capture=True)
        if not result:
            return True
        
        packages = [p.strip() for p in result.split('\n') if p.strip()]
        
        # Also check for akmod-nvidia
        result = self._run_command(['rpm', '-qa', 'akmod-nvidia*'], check=False, capture=True)
        if result:
            akmod_packages = [p.strip() for p in result.split('\n') if p.strip()]
            packages.extend(akmod_packages)
        
        if not packages:
            return True
        
        cmd = ['sudo', 'dnf', 'remove', '-y' if yes else ''] + packages
        cmd = [c for c in cmd if c]
        
        try:
            self._run_command(cmd, check=True)
            self._log_action("driver_remove_nvidia", packages)
            return True
        except RuntimeError:
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
            return True
        except RuntimeError:
            return False

