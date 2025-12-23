use crate::history::History;
use crate::runner::{command, run_capture_allow_fail, run_inherit};
use anyhow::Result;

#[derive(Clone)]
pub struct DriverManager {
    use_sudo: bool,
    history: History,
}

impl DriverManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn detect(&self) -> Result<()> {
        let mut cmd = command("lspci", &[], false);
        let output = run_capture_allow_fail(&mut cmd, "lspci")?.unwrap_or_default();
        let mut found = false;
        for line in output.lines() {
            let upper = line.to_uppercase();
            if upper.contains("NVIDIA") || upper.contains("AMD") || upper.contains("INTEL") {
                println!("{line}");
                found = true;
            }
        }
        if !found {
            println!("No GPUs detected");
        }
        Ok(())
    }

    pub fn status(&self) -> Result<()> {
        println!("=== GPU Detection ===");
        self.detect()?;
        println!("\n=== Nvidia Driver Status ===");
        self.check_nvidia()?;
        Ok(())
    }

    pub fn install_nvidia(&self, version: Option<&str>, cuda: bool, yes: bool) -> Result<()> {
        let mut packages = Vec::new();
        if let Some(v) = version {
            packages.push(format!("akmod-nvidia-{v}"));
        } else {
            packages.push("akmod-nvidia".to_string());
        }
        if cuda {
            packages.push("xorg-x11-drv-nvidia-cuda".into());
            packages.push("xorg-x11-drv-nvidia-cuda-libs".into());
        }
        if !packages.iter().any(|p| p.contains("xorg-x11-drv-nvidia")) {
            packages.push("xorg-x11-drv-nvidia".into());
        }
        println!("Installing Nvidia driver packages: {}", packages.join(", "));
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        args.extend(packages.iter().map(|s| s.as_str()));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install nvidia")?;
        self.history.log("driver_install_nvidia", &packages)?;
        println!("Reboot after installation and verify with `nvidia-smi`.");
        Ok(())
    }

    pub fn remove_nvidia(&self, yes: bool) -> Result<()> {
        let mut cmd = command("rpm", &["-qa", "nvidia*"], false);
        let mut packages: Vec<String> = run_capture_allow_fail(&mut cmd, "rpm -qa nvidia")?
            .unwrap_or_default()
            .lines()
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty())
            .collect();

        let mut cmd = command("rpm", &["-qa", "akmod-nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa akmod-nvidia")? {
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
            println!("No Nvidia packages found");
            return Ok(());
        }

        println!("Removing Nvidia packages: {}", packages.join(", "));
        let mut args = vec!["remove"];
        if yes {
            args.push("-y");
        }
        args.extend(packages.iter().map(|s| s.as_str()));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf remove nvidia")?;
        self.history.log("driver_remove_nvidia", &packages)?;
        Ok(())
    }

    pub fn list_nvidia(&self) -> Result<()> {
        let mut results = Vec::new();
        let mut cmd = command("dnf", &["list", "available", "akmod-nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list akmod-nvidia")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.contains("akmod-nvidia") {
                        results.push(pkg.to_string());
                    }
                }
            }
        }
        let mut cmd = command("dnf", &["list", "available", "nvidia-driver*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list nvidia-driver")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.contains("nvidia-driver") {
                        results.push(pkg.to_string());
                    }
                }
            }
        }
        results.sort();
        results.dedup();
        if results.is_empty() {
            println!("No Nvidia driver packages found (check RPM Fusion repos)");
        } else {
            println!("Available Nvidia drivers ({}):", results.len());
            for r in results.iter().take(20) {
                println!("  {r}");
            }
        }
        Ok(())
    }

    pub fn check_nvidia(&self) -> Result<()> {
        let mut status = command("nvidia-smi", &[], false);
        match run_capture_allow_fail(&mut status, "nvidia-smi")? {
            Some(out) => {
                println!("nvidia-smi output:");
                println!("{out}");
            }
            None => println!("nvidia-smi not available"),
        }
        let mut cmd = command("rpm", &["-qa", "nvidia*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa nvidia")? {
            let packages: Vec<&str> = out.lines().filter(|l| !l.is_empty()).collect();
            if packages.is_empty() {
                println!("No Nvidia packages installed");
            } else {
                println!("Installed Nvidia packages ({}):", packages.len());
                for pkg in packages.iter().take(10) {
                    println!("  {pkg}");
                }
            }
        }
        Ok(())
    }

    pub fn install_cuda(&self, yes: bool) -> Result<()> {
        let packages = vec!["cuda", "cuda-toolkit", "nvidia-cuda-toolkit"];
        println!("Installing CUDA toolchain");
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        args.extend(packages.clone());
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install cuda")?;
        self.history
            .log("driver_install_cuda", &packages.iter().map(|s| s.to_string()).collect::<Vec<_>>())?;
        Ok(())
    }
}

