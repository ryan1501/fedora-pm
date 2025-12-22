#!/usr/bin/env python3
"""
Fedora Package Manager GUI - A modern GUI package manager for Fedora Linux
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox, simpledialog
import threading
import queue
import sys
import os
from pathlib import Path

# Import the package manager class
import sys
import importlib.util
from pathlib import Path

# Try to import from installed location first
try:
    from fedora_pm import FedoraPackageManager
except ImportError:
    # Fallback: try to load from same directory (for development)
    script_dir = Path(__file__).parent
    fedora_pm_path = script_dir / "fedora_pm.py"
    
    if fedora_pm_path.exists():
        spec = importlib.util.spec_from_file_location("fedora_pm", str(fedora_pm_path))
        fedora_pm = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(fedora_pm)
        FedoraPackageManager = fedora_pm.FedoraPackageManager
    else:
        # Try system installation location
        system_path = Path("/usr/share/fedora-pm/fedora_pm.py")
        if system_path.exists():
            spec = importlib.util.spec_from_file_location("fedora_pm", str(system_path))
            fedora_pm = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(fedora_pm)
            FedoraPackageManager = fedora_pm.FedoraPackageManager
        else:
            raise ImportError("Could not find fedora_pm.py module. Please ensure it is installed.")


class FedoraPMGUI:
    """GUI application for Fedora Package Manager"""
    
    def __init__(self, root):
        self.root = root
        self.root.title("Fedora Package Manager")
        self.root.geometry("900x700")
        
        # Create package manager instance
        try:
            self.pm = FedoraPackageManager()
        except SystemExit:
            messagebox.showerror("Error", "Missing required tools (dnf, rpm).\nPlease install them first.")
            sys.exit(1)
        
        # Queue for thread-safe GUI updates
        self.message_queue = queue.Queue()
        self.check_queue()
        
        # Create UI
        self.create_widgets()
        
    def check_queue(self):
        """Check for messages from background threads"""
        try:
            while True:
                msg_type, msg = self.message_queue.get_nowait()
                if msg_type == 'output':
                    self.output_text.insert(tk.END, msg)
                    self.output_text.see(tk.END)
                elif msg_type == 'clear':
                    self.output_text.delete(1.0, tk.END)
                elif msg_type == 'error':
                    messagebox.showerror("Error", msg)
                elif msg_type == 'info':
                    messagebox.showinfo("Information", msg)
                elif msg_type == 'success':
                    messagebox.showinfo("Success", msg)
        except queue.Empty:
            pass
        finally:
            self.root.after(100, self.check_queue)
    
    def log(self, message):
        """Thread-safe logging"""
        self.message_queue.put(('output', message + '\n'))
    
    def create_widgets(self):
        """Create the GUI widgets"""
        # Create notebook for tabs
        notebook = ttk.Notebook(self.root)
        notebook.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Tab 1: Package Management
        self.create_package_tab(notebook)
        
        # Tab 2: Search
        self.create_search_tab(notebook)
        
        # Tab 3: Kernel Management
        self.create_kernel_tab(notebook)
        
        # Tab 4: Driver Management
        self.create_driver_tab(notebook)
        
        # Tab 5: System Operations
        self.create_system_tab(notebook)
        
        # Output area at the bottom
        output_frame = ttk.Frame(self.root)
        output_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        ttk.Label(output_frame, text="Output:", font=('Arial', 10, 'bold')).pack(anchor=tk.W)
        self.output_text = scrolledtext.ScrolledText(output_frame, height=10, wrap=tk.WORD)
        self.output_text.pack(fill=tk.BOTH, expand=True)
        
    def create_package_tab(self, notebook):
        """Create package management tab"""
        frame = ttk.Frame(notebook)
        notebook.add(frame, text="Packages")
        
        # Install section
        install_frame = ttk.LabelFrame(frame, text="Install Packages", padding=10)
        install_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(install_frame, text="Package names (space-separated):").pack(anchor=tk.W)
        self.install_entry = ttk.Entry(install_frame, width=50)
        self.install_entry.pack(fill=tk.X, pady=5)
        
        install_btn = ttk.Button(install_frame, text="Install", command=self.install_packages)
        install_btn.pack(pady=5)
        
        # Remove section
        remove_frame = ttk.LabelFrame(frame, text="Remove Packages", padding=10)
        remove_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(remove_frame, text="Package names (space-separated):").pack(anchor=tk.W)
        self.remove_entry = ttk.Entry(remove_frame, width=50)
        self.remove_entry.pack(fill=tk.X, pady=5)
        
        remove_btn = ttk.Button(remove_frame, text="Remove", command=self.remove_packages)
        remove_btn.pack(pady=5)
        
        # Update section
        update_frame = ttk.LabelFrame(frame, text="Update System", padding=10)
        update_frame.pack(fill=tk.X, padx=5, pady=5)
        
        update_all_btn = ttk.Button(update_frame, text="Update All Packages", command=self.update_all)
        update_all_btn.pack(side=tk.LEFT, padx=5)
        
        ttk.Label(update_frame, text="Or specific packages:").pack(side=tk.LEFT, padx=10)
        self.update_entry = ttk.Entry(update_frame, width=30)
        self.update_entry.pack(side=tk.LEFT, padx=5)
        
        update_btn = ttk.Button(update_frame, text="Update Selected", command=self.update_packages)
        update_btn.pack(side=tk.LEFT, padx=5)
        
        # List section
        list_frame = ttk.LabelFrame(frame, text="List Packages", padding=10)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        list_btn_frame = ttk.Frame(list_frame)
        list_btn_frame.pack(fill=tk.X)
        
        ttk.Button(list_btn_frame, text="List Installed", command=self.list_installed).pack(side=tk.LEFT, padx=5)
        ttk.Button(list_btn_frame, text="List Available", command=self.list_available).pack(side=tk.LEFT, padx=5)
        
        ttk.Label(list_frame, text="Filter (optional):").pack(anchor=tk.W, pady=(10, 5))
        self.list_filter_entry = ttk.Entry(list_frame, width=50)
        self.list_filter_entry.pack(fill=tk.X)
        
        # Package info section
        info_frame = ttk.LabelFrame(frame, text="Package Information", padding=10)
        info_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(info_frame, text="Package name:").pack(anchor=tk.W)
        info_btn_frame = ttk.Frame(info_frame)
        info_btn_frame.pack(fill=tk.X, pady=5)
        
        self.info_entry = ttk.Entry(info_btn_frame, width=40)
        self.info_entry.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(info_btn_frame, text="Get Info", command=self.get_package_info).pack(side=tk.LEFT, padx=5)
    
    def create_search_tab(self, notebook):
        """Create search tab"""
        frame = ttk.Frame(notebook)
        notebook.add(frame, text="Search")
        
        search_frame = ttk.LabelFrame(frame, text="Search Packages", padding=10)
        search_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Label(search_frame, text="Search query:").pack(anchor=tk.W)
        search_btn_frame = ttk.Frame(search_frame)
        search_btn_frame.pack(fill=tk.X, pady=5)
        
        self.search_entry = ttk.Entry(search_btn_frame, width=40)
        self.search_entry.pack(side=tk.LEFT, padx=5)
        self.search_entry.bind('<Return>', lambda e: self.search_packages())
        
        ttk.Button(search_btn_frame, text="Search", command=self.search_packages).pack(side=tk.LEFT, padx=5)
        
        # Results area
        results_frame = ttk.LabelFrame(frame, text="Search Results", padding=10)
        results_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Treeview for results
        columns = ('name', 'description')
        self.search_tree = ttk.Treeview(results_frame, columns=columns, show='tree headings', height=15)
        self.search_tree.heading('#0', text='#')
        self.search_tree.heading('name', text='Package Name')
        self.search_tree.heading('description', text='Description')
        self.search_tree.column('#0', width=50)
        self.search_tree.column('name', width=200)
        self.search_tree.column('description', width=400)
        
        scrollbar = ttk.Scrollbar(results_frame, orient=tk.VERTICAL, command=self.search_tree.yview)
        self.search_tree.configure(yscrollcommand=scrollbar.set)
        
        self.search_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
    
    def create_kernel_tab(self, notebook):
        """Create kernel management tab"""
        frame = ttk.Frame(notebook)
        notebook.add(frame, text="Kernels")
        
        # Current kernel
        current_frame = ttk.LabelFrame(frame, text="Current Kernel", padding=10)
        current_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.current_kernel_label = ttk.Label(current_frame, text="Loading...", font=('Arial', 10, 'bold'))
        self.current_kernel_label.pack()
        
        ttk.Button(current_frame, text="Refresh", command=self.refresh_current_kernel).pack(pady=5)
        
        # List kernels
        list_frame = ttk.LabelFrame(frame, text="Installed Kernels", padding=10)
        list_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        ttk.Button(list_frame, text="List Installed Kernels", command=self.list_kernels).pack(pady=5)
        
        # Kernel treeview
        kernel_columns = ('version', 'package', 'type', 'current')
        self.kernel_tree = ttk.Treeview(list_frame, columns=kernel_columns, show='tree headings', height=10)
        self.kernel_tree.heading('#0', text='#')
        self.kernel_tree.heading('version', text='Version')
        self.kernel_tree.heading('package', text='Package')
        self.kernel_tree.heading('type', text='Type')
        self.kernel_tree.heading('current', text='Current')
        self.kernel_tree.column('#0', width=50)
        self.kernel_tree.column('version', width=150)
        self.kernel_tree.column('package', width=200)
        self.kernel_tree.column('type', width=100)
        self.kernel_tree.column('current', width=80)
        
        kernel_scrollbar = ttk.Scrollbar(list_frame, orient=tk.VERTICAL, command=self.kernel_tree.yview)
        self.kernel_tree.configure(yscrollcommand=kernel_scrollbar.set)
        
        self.kernel_tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        kernel_scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Kernel actions
        actions_frame = ttk.LabelFrame(frame, text="Kernel Actions", padding=10)
        actions_frame.pack(fill=tk.X, padx=5, pady=5)
        
        btn_frame = ttk.Frame(actions_frame)
        btn_frame.pack()
        
        ttk.Button(btn_frame, text="Install Latest", command=self.install_latest_kernel).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame, text="Remove Old Kernels", command=self.remove_old_kernels).pack(side=tk.LEFT, padx=5)
        
        # CachyOS section
        cachyos_frame = ttk.LabelFrame(frame, text="CachyOS Kernels", padding=10)
        cachyos_frame.pack(fill=tk.X, padx=5, pady=5)
        
        cachyos_btn_frame = ttk.Frame(cachyos_frame)
        cachyos_btn_frame.pack()
        
        ttk.Button(cachyos_btn_frame, text="Check Repos", command=self.check_cachyos_repos).pack(side=tk.LEFT, padx=5)
        ttk.Button(cachyos_btn_frame, text="List Available", command=self.list_cachyos_kernels).pack(side=tk.LEFT, padx=5)
        ttk.Button(cachyos_btn_frame, text="Check CPU Support", command=self.check_cpu_support).pack(side=tk.LEFT, padx=5)
        
        # Refresh on load
        self.refresh_current_kernel()
    
    def create_driver_tab(self, notebook):
        """Create driver management tab"""
        frame = ttk.Frame(notebook)
        notebook.add(frame, text="Drivers")
        
        # Status section
        status_frame = ttk.LabelFrame(frame, text="Driver Status", padding=10)
        status_frame.pack(fill=tk.X, padx=5, pady=5)
        
        ttk.Button(status_frame, text="Check Driver Status", command=self.check_driver_status).pack(pady=5)
        
        self.driver_status_text = scrolledtext.ScrolledText(status_frame, height=10, wrap=tk.WORD)
        self.driver_status_text.pack(fill=tk.BOTH, expand=True, pady=5)
        
        # Nvidia section
        nvidia_frame = ttk.LabelFrame(frame, text="Nvidia Drivers", padding=10)
        nvidia_frame.pack(fill=tk.X, padx=5, pady=5)
        
        nvidia_btn_frame = ttk.Frame(nvidia_frame)
        nvidia_btn_frame.pack()
        
        ttk.Button(nvidia_btn_frame, text="Detect GPU", command=self.detect_gpu).pack(side=tk.LEFT, padx=5)
        ttk.Button(nvidia_btn_frame, text="Check Nvidia", command=self.check_nvidia).pack(side=tk.LEFT, padx=5)
        ttk.Button(nvidia_btn_frame, text="Install Nvidia", command=self.install_nvidia).pack(side=tk.LEFT, padx=5)
        ttk.Button(nvidia_btn_frame, text="Remove Nvidia", command=self.remove_nvidia).pack(side=tk.LEFT, padx=5)
        
        cuda_frame = ttk.Frame(nvidia_frame)
        cuda_frame.pack(pady=5)
        
        ttk.Button(cuda_frame, text="Install CUDA Toolkit", command=self.install_cuda).pack(side=tk.LEFT, padx=5)
    
    def create_system_tab(self, notebook):
        """Create system operations tab"""
        frame = ttk.Frame(notebook)
        notebook.add(frame, text="System")
        
        # Clean section
        clean_frame = ttk.LabelFrame(frame, text="Clean Cache", padding=10)
        clean_frame.pack(fill=tk.X, padx=5, pady=5)
        
        self.clean_cache_var = tk.BooleanVar(value=True)
        self.clean_metadata_var = tk.BooleanVar(value=True)
        
        ttk.Checkbutton(clean_frame, text="Clean package cache", variable=self.clean_cache_var).pack(anchor=tk.W)
        ttk.Checkbutton(clean_frame, text="Clean metadata", variable=self.clean_metadata_var).pack(anchor=tk.W)
        
        ttk.Button(clean_frame, text="Clean", command=self.clean_system).pack(pady=5)
        
        # History section
        history_frame = ttk.LabelFrame(frame, text="History", padding=10)
        history_frame.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        history_btn_frame = ttk.Frame(history_frame)
        history_btn_frame.pack(fill=tk.X, pady=5)
        
        ttk.Label(history_btn_frame, text="Show last:").pack(side=tk.LEFT, padx=5)
        self.history_limit = ttk.Spinbox(history_btn_frame, from_=1, to=100, width=10, value=10)
        self.history_limit.pack(side=tk.LEFT, padx=5)
        
        ttk.Button(history_btn_frame, text="Show History", command=self.show_history).pack(side=tk.LEFT, padx=5)
        
        self.history_text = scrolledtext.ScrolledText(history_frame, height=15, wrap=tk.WORD)
        self.history_text.pack(fill=tk.BOTH, expand=True)
    
    # Package management methods
    def install_packages(self):
        """Install packages"""
        packages = self.install_entry.get().strip().split()
        if not packages:
            messagebox.showwarning("Warning", "Please enter package names")
            return
        
        if messagebox.askyesno("Confirm", f"Install packages: {', '.join(packages)}?"):
            threading.Thread(target=self._install_packages_thread, args=(packages,), daemon=True).start()
    
    def _install_packages_thread(self, packages):
        """Thread for installing packages"""
        self.message_queue.put(('clear', ''))
        self.log(f"Installing packages: {', '.join(packages)}...")
        try:
            result = self.pm.install(packages, yes=True)
            if result:
                self.message_queue.put(('success', f"Successfully installed: {', '.join(packages)}"))
            else:
                self.message_queue.put(('error', "Installation failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def remove_packages(self):
        """Remove packages"""
        packages = self.remove_entry.get().strip().split()
        if not packages:
            messagebox.showwarning("Warning", "Please enter package names")
            return
        
        if messagebox.askyesno("Confirm", f"Remove packages: {', '.join(packages)}?"):
            threading.Thread(target=self._remove_packages_thread, args=(packages,), daemon=True).start()
    
    def _remove_packages_thread(self, packages):
        """Thread for removing packages"""
        self.message_queue.put(('clear', ''))
        self.log(f"Removing packages: {', '.join(packages)}...")
        try:
            result = self.pm.remove(packages, yes=True)
            if result:
                self.message_queue.put(('success', f"Successfully removed: {', '.join(packages)}"))
            else:
                self.message_queue.put(('error', "Removal failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def update_all(self):
        """Update all packages"""
        if messagebox.askyesno("Confirm", "Update all packages?"):
            threading.Thread(target=self._update_all_thread, daemon=True).start()
    
    def _update_all_thread(self):
        """Thread for updating all packages"""
        self.message_queue.put(('clear', ''))
        self.log("Updating all packages...")
        try:
            result = self.pm.update(yes=True)
            if result:
                self.message_queue.put(('success', "System updated successfully"))
            else:
                self.message_queue.put(('error', "Update failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def update_packages(self):
        """Update specific packages"""
        packages = self.update_entry.get().strip().split()
        if not packages:
            messagebox.showwarning("Warning", "Please enter package names")
            return
        
        if messagebox.askyesno("Confirm", f"Update packages: {', '.join(packages)}?"):
            threading.Thread(target=self._update_packages_thread, args=(packages,), daemon=True).start()
    
    def _update_packages_thread(self, packages):
        """Thread for updating packages"""
        self.message_queue.put(('clear', ''))
        self.log(f"Updating packages: {', '.join(packages)}...")
        try:
            result = self.pm.update(packages, yes=True)
            if result:
                self.message_queue.put(('success', f"Successfully updated: {', '.join(packages)}"))
            else:
                self.message_queue.put(('error', "Update failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def list_installed(self):
        """List installed packages"""
        pattern = self.list_filter_entry.get().strip() or None
        threading.Thread(target=self._list_installed_thread, args=(pattern,), daemon=True).start()
    
    def _list_installed_thread(self, pattern):
        """Thread for listing installed packages"""
        self.message_queue.put(('clear', ''))
        self.log("Listing installed packages...")
        try:
            packages = self.pm.list_installed(pattern)
            self.log(f"\nFound {len(packages)} installed packages:\n")
            for pkg in packages[:100]:  # Limit output
                self.log(f"  {pkg}")
            if len(packages) > 100:
                self.log(f"\n... and {len(packages) - 100} more packages")
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def list_available(self):
        """List available packages"""
        pattern = self.list_filter_entry.get().strip() or None
        threading.Thread(target=self._list_available_thread, args=(pattern,), daemon=True).start()
    
    def _list_available_thread(self, pattern):
        """Thread for listing available packages"""
        self.message_queue.put(('clear', ''))
        self.log("Listing available packages...")
        try:
            packages = self.pm.list_available(pattern)
            self.log(f"\nFound {len(packages)} available packages:\n")
            for pkg in packages[:100]:  # Limit output
                self.log(f"  {pkg}")
            if len(packages) > 100:
                self.log(f"\n... and {len(packages) - 100} more packages")
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def get_package_info(self):
        """Get package information"""
        package = self.info_entry.get().strip()
        if not package:
            messagebox.showwarning("Warning", "Please enter a package name")
            return
        
        threading.Thread(target=self._get_package_info_thread, args=(package,), daemon=True).start()
    
    def _get_package_info_thread(self, package):
        """Thread for getting package info"""
        self.message_queue.put(('clear', ''))
        self.log(f"Getting information for package: {package}...")
        try:
            info = self.pm.info(package)
            if info:
                self.log(f"\nPackage Information:\n{info.get('info', 'No info available')}")
            else:
                self.message_queue.put(('error', f"Package '{package}' not found"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    # Search methods
    def search_packages(self):
        """Search for packages"""
        query = self.search_entry.get().strip()
        if not query:
            messagebox.showwarning("Warning", "Please enter a search query")
            return
        
        threading.Thread(target=self._search_packages_thread, args=(query,), daemon=True).start()
    
    def _search_packages_thread(self, query):
        """Thread for searching packages"""
        # Clear results
        for item in self.search_tree.get_children():
            self.search_tree.delete(item)
        
        self.message_queue.put(('clear', ''))
        self.log(f"Searching for: {query}...")
        try:
            packages = self.pm.search(query)
            self.log(f"\nFound {len(packages)} packages\n")
            
            for idx, pkg in enumerate(packages[:50], 1):  # Limit to 50 results
                self.search_tree.insert('', tk.END, text=str(idx), 
                                       values=(pkg['name'], pkg['description']))
                self.log(f"  {pkg['name']}: {pkg['description']}")
            
            if len(packages) > 50:
                self.log(f"\n... and {len(packages) - 50} more results")
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    # Kernel methods
    def refresh_current_kernel(self):
        """Refresh current kernel display"""
        threading.Thread(target=self._refresh_current_kernel_thread, daemon=True).start()
    
    def _refresh_current_kernel_thread(self):
        """Thread for refreshing current kernel"""
        try:
            kernel = self.pm.kernel_current()
            if kernel:
                self.current_kernel_label.config(text=f"Current Kernel: {kernel}")
            else:
                self.current_kernel_label.config(text="Current Kernel: Unknown")
        except Exception as e:
            self.current_kernel_label.config(text="Error loading kernel info")
    
    def list_kernels(self):
        """List installed kernels"""
        threading.Thread(target=self._list_kernels_thread, daemon=True).start()
    
    def _list_kernels_thread(self):
        """Thread for listing kernels"""
        # Clear tree
        for item in self.kernel_tree.get_children():
            self.kernel_tree.delete(item)
        
        try:
            kernels = self.pm.kernel_list_installed()
            for idx, kernel in enumerate(kernels, 1):
                current = "Yes" if kernel.get('is_current') else "No"
                self.kernel_tree.insert('', tk.END, text=str(idx),
                                       values=(kernel['version'], kernel['package'], 
                                              kernel.get('type', 'standard'), current))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def install_latest_kernel(self):
        """Install latest kernel"""
        if messagebox.askyesno("Confirm", "Install latest kernel?"):
            threading.Thread(target=self._install_latest_kernel_thread, daemon=True).start()
    
    def _install_latest_kernel_thread(self):
        """Thread for installing latest kernel"""
        self.message_queue.put(('clear', ''))
        self.log("Installing latest kernel...")
        try:
            result = self.pm.kernel_install(yes=True)
            if result:
                self.message_queue.put(('success', "Kernel installed successfully. Reboot to use it."))
                self.list_kernels()
            else:
                self.message_queue.put(('error', "Kernel installation failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def remove_old_kernels(self):
        """Remove old kernels"""
        keep = simpledialog.askinteger("Remove Old Kernels", "How many kernels to keep?", initialvalue=2, minvalue=1, maxvalue=10)
        if keep:
            if messagebox.askyesno("Confirm", f"Remove old kernels (keeping {keep} newest)?"):
                threading.Thread(target=self._remove_old_kernels_thread, args=(keep,), daemon=True).start()
    
    def _remove_old_kernels_thread(self, keep):
        """Thread for removing old kernels"""
        self.message_queue.put(('clear', ''))
        self.log(f"Removing old kernels (keeping {keep} newest)...")
        try:
            result = self.pm.kernel_remove_old(keep=keep, yes=True)
            if result:
                self.message_queue.put(('success', "Old kernels removed successfully"))
                self.list_kernels()
            else:
                self.message_queue.put(('info', "No old kernels to remove"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def check_cachyos_repos(self):
        """Check CachyOS repositories"""
        threading.Thread(target=self._check_cachyos_repos_thread, daemon=True).start()
    
    def _check_cachyos_repos_thread(self):
        """Thread for checking CachyOS repos"""
        try:
            repos = self.pm.cachyos_check_repo()
            msg = "CachyOS Repository Status:\n"
            msg += f"GCC Repository: {'✓ Enabled' if repos['gcc'] else '✗ Not enabled'}\n"
            msg += f"LTO Repository: {'✓ Enabled' if repos['lto'] else '✗ Not enabled'}"
            self.message_queue.put(('info', msg))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def list_cachyos_kernels(self):
        """List available CachyOS kernels"""
        threading.Thread(target=self._list_cachyos_kernels_thread, daemon=True).start()
    
    def _list_cachyos_kernels_thread(self):
        """Thread for listing CachyOS kernels"""
        self.message_queue.put(('clear', ''))
        self.log("Listing available CachyOS kernels...")
        try:
            kernels = self.pm.cachyos_list_available()
            if kernels:
                self.log(f"\nFound {len(kernels)} CachyOS kernels:\n")
                for kernel in kernels:
                    self.log(f"  {kernel['package']} ({kernel['build'].upper()} build, type: {kernel['type']})")
            else:
                self.log("No CachyOS kernels found or repositories not enabled")
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def check_cpu_support(self):
        """Check CPU instruction set support"""
        threading.Thread(target=self._check_cpu_support_thread, daemon=True).start()
    
    def _check_cpu_support_thread(self):
        """Thread for checking CPU support"""
        try:
            support = self.pm.cachyos_check_cpu_support()
            msg = "CPU Instruction Set Support:\n"
            for isa, supported in support.items():
                status = "✓ Supported" if supported else "✗ Not supported"
                msg += f"{isa}: {status}\n"
            self.message_queue.put(('info', msg))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    # Driver methods
    def check_driver_status(self):
        """Check driver status"""
        threading.Thread(target=self._check_driver_status_thread, daemon=True).start()
    
    def _check_driver_status_thread(self):
        """Thread for checking driver status"""
        self.driver_status_text.delete(1.0, tk.END)
        try:
            status = self.pm.driver_status()
            output = "=== Driver Status ===\n\n"
            
            if 'gpus' in status and status['gpus']:
                output += "Detected GPUs:\n"
                for gpu_type, gpu_info in status['gpus'].items():
                    output += f"  {gpu_type.upper()}: {gpu_info}\n"
                output += "\n"
            
            if 'nvidia' in status:
                nvidia = status['nvidia']
                output += "Nvidia Driver:\n"
                if nvidia.get('installed'):
                    output += "  Status: ✓ Installed\n"
                    if 'version' in nvidia:
                        output += f"  Version: {nvidia['version']}\n"
                else:
                    output += "  Status: ✗ Not installed\n"
                output += "\n"
            
            self.driver_status_text.insert(tk.END, output)
        except Exception as e:
            self.driver_status_text.insert(tk.END, f"Error: {str(e)}")
    
    def detect_gpu(self):
        """Detect GPU hardware"""
        threading.Thread(target=self._detect_gpu_thread, daemon=True).start()
    
    def _detect_gpu_thread(self):
        """Thread for detecting GPU"""
        try:
            gpus = self.pm.driver_detect_gpu()
            if gpus:
                msg = "Detected GPUs:\n"
                for gpu_type, gpu_info in gpus.items():
                    msg += f"{gpu_type.upper()}: {gpu_info}\n"
                self.message_queue.put(('info', msg))
            else:
                self.message_queue.put(('info', "No GPUs detected"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def check_nvidia(self):
        """Check Nvidia driver status"""
        threading.Thread(target=self._check_nvidia_thread, daemon=True).start()
    
    def _check_nvidia_thread(self):
        """Thread for checking Nvidia"""
        self.message_queue.put(('clear', ''))
        self.log("Checking Nvidia driver status...")
        try:
            status = self.pm.driver_check_nvidia()
            if status.get('installed'):
                self.log("Status: ✓ Installed")
                if 'version' in status:
                    self.log(f"Driver Version: {status['version']}")
            else:
                self.log("Status: ✗ Not installed")
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def install_nvidia(self):
        """Install Nvidia drivers"""
        if messagebox.askyesno("Confirm", "Install Nvidia drivers?\n\nNote: This requires RPM Fusion repositories."):
            cuda = messagebox.askyesno("CUDA", "Also install CUDA support?")
            threading.Thread(target=self._install_nvidia_thread, args=(cuda,), daemon=True).start()
    
    def _install_nvidia_thread(self, cuda):
        """Thread for installing Nvidia"""
        self.message_queue.put(('clear', ''))
        self.log("Installing Nvidia drivers...")
        try:
            result = self.pm.driver_install_nvidia(cuda=cuda, yes=True)
            if result:
                self.message_queue.put(('success', "Nvidia drivers installed. Reboot required."))
            else:
                self.message_queue.put(('error', "Nvidia driver installation failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def remove_nvidia(self):
        """Remove Nvidia drivers"""
        if messagebox.askyesno("Confirm", "Remove Nvidia drivers?"):
            threading.Thread(target=self._remove_nvidia_thread, daemon=True).start()
    
    def _remove_nvidia_thread(self):
        """Thread for removing Nvidia"""
        self.message_queue.put(('clear', ''))
        self.log("Removing Nvidia drivers...")
        try:
            result = self.pm.driver_remove_nvidia(yes=True)
            if result:
                self.message_queue.put(('success', "Nvidia drivers removed. Reboot may be required."))
            else:
                self.message_queue.put(('info', "No Nvidia drivers found to remove"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def install_cuda(self):
        """Install CUDA toolkit"""
        if messagebox.askyesno("Confirm", "Install CUDA toolkit?"):
            threading.Thread(target=self._install_cuda_thread, daemon=True).start()
    
    def _install_cuda_thread(self):
        """Thread for installing CUDA"""
        self.message_queue.put(('clear', ''))
        self.log("Installing CUDA toolkit...")
        try:
            result = self.pm.driver_install_cuda(yes=True)
            if result:
                self.message_queue.put(('success', "CUDA toolkit installed successfully"))
            else:
                self.message_queue.put(('error', "CUDA installation failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    # System methods
    def clean_system(self):
        """Clean system cache"""
        cache = self.clean_cache_var.get()
        metadata = self.clean_metadata_var.get()
        
        if not cache and not metadata:
            messagebox.showwarning("Warning", "Please select at least one option")
            return
        
        if messagebox.askyesno("Confirm", "Clean system cache?"):
            threading.Thread(target=self._clean_system_thread, args=(cache, metadata), daemon=True).start()
    
    def _clean_system_thread(self, cache, metadata):
        """Thread for cleaning system"""
        self.message_queue.put(('clear', ''))
        self.log("Cleaning system...")
        try:
            result = self.pm.clean(cache=cache, metadata=metadata)
            if result:
                self.message_queue.put(('success', "System cleaned successfully"))
            else:
                self.message_queue.put(('error', "Clean failed"))
        except Exception as e:
            self.message_queue.put(('error', str(e)))
    
    def show_history(self):
        """Show package management history"""
        limit = int(self.history_limit.get())
        threading.Thread(target=self._show_history_thread, args=(limit,), daemon=True).start()
    
    def _show_history_thread(self, limit):
        """Thread for showing history"""
        self.history_text.delete(1.0, tk.END)
        try:
            history = self.pm.history(limit=limit)
            if history:
                output = f"Package Management History (last {len(history)} entries):\n\n"
                for entry in reversed(history):
                    output += f"[{entry['timestamp']}] {entry['action']}: {', '.join(entry['packages'])}\n"
                self.history_text.insert(tk.END, output)
            else:
                self.history_text.insert(tk.END, "No history found")
        except Exception as e:
            self.history_text.insert(tk.END, f"Error: {str(e)}")


def main():
    """Main entry point for GUI"""
    root = tk.Tk()
    app = FedoraPMGUI(root)
    root.mainloop()


if __name__ == '__main__':
    main()

