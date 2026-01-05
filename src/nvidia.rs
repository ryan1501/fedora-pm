use crate::history::History;
use crate::runner::{command, run_capture, run_capture_allow_fail, run_inherit};
use anyhow::Result;

pub struct NvidiaManager {
    use_sudo: bool,
    history: History,
}

impl NvidiaManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn detect_hardware(&self) -> Result<()> {
        println!("=== NVIDIA Hardware Detection ===");
        
        let mut cmd = command("lspci", &["-nn", "|", "grep", "-i", "nvidia"], false);
        if let Some(output) = run_capture_allow_fail(&mut cmd, "lspci nvidia")? {
            if !output.trim().is_empty() {
                println!("NVIDIA GPUs detected:");
                for line in output.lines() {
                    println!("  {}", line.trim());
                }
                return Ok(());
            }
        }

        println!("No NVIDIA GPUs detected");
        Ok(())
    }

    pub fn check_driver_status(&self) -> Result<()> {
        println!("=== NVIDIA Driver Status ===");
        
        // Check nvidia-smi
        let mut smi_cmd = command("nvidia-smi", &[], false);
        match run_capture_allow_fail(&mut smi_cmd, "nvidia-smi")? {
            Some(output) => {
                println!("✓ nvidia-smi is working");
                println!("{}", output);
            }
            None => {
                println!("✗ nvidia-smi not available");
            }
        }

        // Check installed packages
        self.check_installed_packages()?;
        
        // Check kernel module
        self.check_kernel_module()?;
        
        Ok(())
    }

    pub fn list_available_drivers(&self) -> Result<()> {
        println!("=== Available NVIDIA Drivers ===");
        
        let mut drivers = Vec::new();
        
        // Check RPM Fusion drivers
        let mut cmd = command("dnf", &["list", "available", "akmod-nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list akmod-nvidia")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.starts_with("akmod-nvidia") {
                        drivers.push((pkg.to_string(), "RPM Fusion".to_string()));
                    }
                }
            }
        }
        
        // Check proprietary drivers
        let mut cmd = command("dnf", &["list", "available", "nvidia-driver*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list nvidia-driver")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.starts_with("nvidia-driver") && !pkg.contains("cuda") {
                        drivers.push((pkg.to_string(), "Proprietary".to_string()));
                    }
                }
            }
        }
        
        if drivers.is_empty() {
            println!("No NVIDIA drivers found.");
            println!("Make sure RPM Fusion repositories are enabled:");
            println!("  sudo dnf install fedora-workstation-repositories");
            println!("  sudo dnf config-manager --enable rpmfusion-nonfree-nvidia-driver");
            println!("  sudo dnf config-manager --enable rpmfusion-nonfree-updates-testing");
        } else {
            println!("Available drivers:");
            for (pkg, source) in drivers {
                let version = self.extract_version(&pkg);
                println!("  {} - {} ({})", pkg, version.unwrap_or("Unknown".to_string()), source);
            }
        }
        
        Ok(())
    }

    pub fn install_driver(&self, version: Option<&str>, cuda: bool, toolkit: bool, yes: bool) -> Result<()> {
        // Ensure RPM Fusion repos are enabled
        self.ensure_rpmfusion_repos(yes)?;
        
        let mut packages = Vec::new();
        
        if let Some(v) = version {
            packages.push(format!("akmod-nvidia-{}xx", v));
            packages.push(format!("xorg-x11-drv-nvidia-{}xx", v));
            packages.push(format!("xorg-x11-drv-nvidia-{}xx-cuda", v));
        } else {
            packages.push("akmod-nvidia".to_string());
            packages.push("xorg-x11-drv-nvidia".to_string());
            packages.push("xorg-x11-drv-nvidia-cuda".to_string());
        }
        
        if cuda {
            packages.extend(vec![
                "cuda".to_string(),
                "cuda-toolkit".to_string(),
            ]);
        }
        
        if toolkit {
            packages.extend(vec![
                "nvidia-cuda-toolkit".to_string(),
                "nvidia-cuda-devel".to_string(),
                "nvidia-cuda-doc".to_string(),
            ]);
        }
        
        println!("Installing NVIDIA driver packages...");
        println!("Packages: {}", packages.join(", "));
        
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for pkg in &packages {
            args.push(pkg);
        }
        
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install nvidia driver")?;
        
        // Configure X11
        self.configure_x11(yes)?;
        
        self.history.log("nvidia_install", &packages)?;
        
        println!("✓ NVIDIA driver installation completed");
        println!("Reboot to apply changes");
        println!("After reboot, verify with: nvidia-smi");
        
        Ok(())
    }

    pub fn remove_driver(&self, yes: bool) -> Result<()> {
        let mut packages = Vec::new();
        
        // Get all nvidia packages
        let mut cmd = command("rpm", &["-qa", "nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa nvidia")? {
            for pkg in out.lines() {
                let pkg = pkg.trim();
                if !pkg.is_empty() && !pkg.contains("firmware") {
                    packages.push(pkg.to_string());
                }
            }
        }
        
        let mut cmd = command("rpm", &["-qa", "akmod-nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa akmod-nvidia")? {
            for pkg in out.lines() {
                let pkg = pkg.trim();
                if !pkg.is_empty() {
                    packages.push(pkg.to_string());
                }
            }
        }
        
        let mut cmd = command("rpm", &["-qa", "cuda*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa cuda")? {
            for pkg in out.lines() {
                let pkg = pkg.trim();
                if !pkg.is_empty() {
                    packages.push(pkg.to_string());
                }
            }
        }
        
        packages.sort();
        packages.dedup();
        
        if packages.is_empty() {
            println!("No NVIDIA packages found to remove");
            return Ok(());
        }
        
        println!("Removing NVIDIA packages: {}", packages.join(", "));
        
        let mut args = vec!["remove"];
        if yes {
            args.push("-y");
        }
        for pkg in &packages {
            args.push(pkg);
        }
        
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf remove nvidia")?;
        
        self.history.log("nvidia_remove", &packages)?;
        
        println!("✓ NVIDIA driver removed");
        println!("Reboot to apply changes");
        
        Ok(())
    }

    pub fn check_cuda_support(&self) -> Result<()> {
        println!("=== CUDA Support Check ===");
        
        // Check if nvidia-smi works
        let mut smi_cmd = command("nvidia-smi", &[], false);
        match run_capture_allow_fail(&mut smi_cmd, "nvidia-smi")? {
            Some(output) => {
                println!("✓ NVIDIA driver is working");
                
                // Extract CUDA version from nvidia-smi
                for line in output.lines() {
                    if line.contains("CUDA Version") {
                        if let Some(cuda_version) = line.split(':').nth(1) {
                            println!("CUDA Version:{}", cuda_version.trim());
                        }
                    }
                }
            }
            None => {
                println!("✗ NVIDIA driver not working - CUDA unavailable");
                return Ok(());
            }
        }
        
        // Check nvcc
        let mut nvcc_cmd = command("nvcc", &["--version"], false);
        match run_capture_allow_fail(&mut nvcc_cmd, "nvcc --version")? {
            Some(output) => {
                println!("✓ CUDA compiler (nvcc) found");
                for line in output.lines() {
                    if line.contains("release") {
                        println!("  {}", line.trim());
                    }
                }
            }
            None => {
                println!("✗ CUDA compiler (nvcc) not found");
                println!("Install with: fedora-pm driver install-nvidia --cuda --toolkit");
            }
        }
        
        // Check CUDA libraries
        let cuda_paths = vec![
            "/usr/local/cuda",
            "/usr/cuda",
            "/opt/cuda",
        ];
        
        println!("\nCUDA installation paths:");
        for path in cuda_paths {
            if std::path::Path::new(path).exists() {
                println!("  ✓ {}", path);
            } else {
                println!("  ✗ {}", path);
            }
        }
        
        Ok(())
    }

    pub fn setup_development(&self, yes: bool) -> Result<()> {
        println!("Setting up NVIDIA development environment...");
        
        let packages = vec![
            "nvidia-cuda-toolkit".to_string(),
            "nvidia-cuda-devel".to_string(),
            "nvidia-cuda-doc".to_string(),
            "cuda-samples".to_string(),
            "cuda-profiler-tools".to_string(),
            "nsight-compute".to_string(),
            "nsight-systems".to_string(),
        ];
        
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for pkg in &packages {
            args.push(pkg);
        }
        
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install cuda development")?;
        
        self.history.log("nvidia_dev_setup", &packages)?;
        
        println!("✓ NVIDIA development environment set up");
        println!("Compile samples with: /usr/local/cuda/samples/1_Utilities/deviceQuery/deviceQuery");
        
        Ok(())
    }

    fn ensure_rpmfusion_repos(&self, yes: bool) -> Result<()> {
        let mut cmd = command("dnf", &["repolist", "enabled"], false);
        let output = run_capture(&mut cmd, "dnf repolist")?;
        
        if !output.contains("rpmfusion") {
            println!("Enabling RPM Fusion repositories...");
            
            let packages = vec![
                "fedora-workstation-repositories".to_string(),
            ];
            
            let mut args = vec!["install"];
            if yes {
                args.push("-y");
            }
            for pkg in &packages {
                args.push(pkg);
            }
            
            let mut cmd = command("dnf", &args, self.use_sudo);
            run_inherit(&mut cmd, "dnf install rpmfusion")?;
            
            // Enable NVIDIA repos
            let repos = vec![
                "rpmfusion-nonfree-nvidia-driver",
                "rpmfusion-nonfree-updates-testing",
            ];
            
            for repo in repos {
                let mut cmd = command("dnf", &["config-manager", "--enable", repo], self.use_sudo);
                run_inherit(&mut cmd, "dnf enable nvidia repo")?;
            }
            
            println!("✓ RPM Fusion repositories enabled");
        }
        
        Ok(())
    }

    fn configure_x11(&self, _yes: bool) -> Result<()> {
        println!("Configuring X11 for NVIDIA...");
        
        // Create xorg.conf if needed
        let mut cmd = command("nvidia-xconfig", &[], self.use_sudo);
        let _ = run_capture_allow_fail(&mut cmd, "nvidia-xconfig");
        
        // Update alternatives for libglvnd
        let mut cmd = command("alternatives", &["--set", "libglvnd", "/usr/share/glvnd/egl_vendor.d/10_nvidia.json"], self.use_sudo);
        let _ = run_capture_allow_fail(&mut cmd, "alternatives libglvnd");
        
        println!("✓ X11 configuration updated");
        
        Ok(())
    }

    fn check_installed_packages(&self) -> Result<()> {
        let mut cmd = command("rpm", &["-qa", "nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa nvidia")? {
            let packages: Vec<&str> = out.lines()
                .filter(|l| !l.is_empty())
                .collect();
            
            if packages.is_empty() {
                println!("✗ No NVIDIA packages installed");
            } else {
                println!("Installed NVIDIA packages ({}):", packages.len());
                for pkg in packages.iter().take(10) {
                    println!("  {}", pkg);
                }
            }
        }
        
        Ok(())
    }

    fn check_kernel_module(&self) -> Result<()> {
        let mut cmd = command("lsmod", &[], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "lsmod")? {
            if out.contains("nvidia") {
                println!("✓ NVIDIA kernel module loaded");
                
                // Show module details
                for line in out.lines() {
                    if line.starts_with("nvidia") {
                        println!("  {}", line);
                    }
                }
            } else {
                println!("✗ NVIDIA kernel module not loaded");
            }
        }
        
        Ok(())
    }

    fn extract_version(&self, package: &str) -> Option<String> {
        // Extract version from package name like "akmod-nvidia-470xx" -> "470"
        if let Some(captures) = regex::Regex::new(r"nvidia-(\d+)xx").ok()?.captures(package) {
            captures.get(1).map(|m| m.as_str().to_string())
        } else if let Some(captures) = regex::Regex::new(r"nvidia-driver-(\d+)").ok()?.captures(package) {
            captures.get(1).map(|m| m.as_str().to_string())
        } else {
            None
        }
    }
}