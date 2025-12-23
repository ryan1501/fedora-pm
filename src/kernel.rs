use std::collections::BTreeMap;

use crate::history::History;
use crate::runner::{command, run_capture, run_capture_allow_fail, run_inherit};
use anyhow::Result;

#[derive(Clone)]
pub struct KernelManager {
    use_sudo: bool,
    history: History,
}

impl KernelManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn current(&self) -> Result<()> {
        let mut cmd = command("uname", &["-r"], false);
        let output = run_capture(&mut cmd, "uname -r")?;
        println!("Current kernel: {}", output.trim());
        Ok(())
    }

    pub fn list_installed(&self) -> Result<Vec<String>> {
        let mut cmd = command("rpm", &["-qa", "kernel*"], false);
        let output = run_capture(&mut cmd, "rpm -qa kernel*")?;
        let mut kernels: Vec<String> = output
            .lines()
            .filter(|l| l.starts_with("kernel"))
            .map(|s| s.trim().to_string())
            .collect();
        kernels.sort();
        kernels.dedup();
        if kernels.is_empty() {
            println!("No kernels installed");
        } else {
            println!("Installed kernels ({}):", kernels.len());
            for k in &kernels {
                println!("  {k}");
            }
        }
        Ok(kernels)
    }

    pub fn list_available(&self) -> Result<Vec<String>> {
        let mut cmd = command("dnf", &["list", "available", "kernel*"], false);
        let output = run_capture(&mut cmd, "dnf list available kernel*")?;
        let kernels: Vec<String> = output
            .lines()
            .skip(1)
            .filter_map(|l| l.split_whitespace().next())
            .filter(|name| name.starts_with("kernel-") && !name.contains("core"))
            .map(|s| s.to_string())
            .collect();
        if kernels.is_empty() {
            println!("No available kernels found");
        } else {
            println!("Available kernels ({}):", kernels.len());
            for k in kernels.iter().take(20) {
                println!("  {k}");
            }
        }
        Ok(kernels)
    }

    pub fn install(&self, version: Option<&str>, yes: bool) -> Result<()> {
        let package = if let Some(v) = version {
            format!("kernel-{v}")
        } else {
            "kernel".to_string()
        };
        println!("Installing kernel package: {package}");
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        args.push(&package);
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install kernel")?;
        self.history.log("kernel_install", &[package])?;
        println!("Reboot to use the new kernel");
        Ok(())
    }

    pub fn remove(&self, versions: &[String], yes: bool, keep_current: bool) -> Result<()> {
        if versions.is_empty() {
            anyhow::bail!("no kernel versions specified");
        }
        let current = if keep_current {
            let mut cmd = command("uname", &["-r"], false);
            run_capture_allow_fail(&mut cmd, "uname -r")?
                .map(|s| s.trim().to_string())
        } else {
            None
        };

        let mut packages = Vec::new();
        for version in versions {
            if let Some(ref cur) = current {
                if cur.contains(version) {
                    println!("Skipping current kernel {version}");
                    continue;
                }
            }
            let query = format!("kernel*{version}*");
            let mut cmd = command("rpm", &["-qa", &query], false);
            if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa kernel")? {
                for pkg in out.lines().map(|s| s.trim()).filter(|l| !l.is_empty()) {
                    packages.push(pkg.to_string());
                }
            }
        }

        packages.sort();
        packages.dedup();
        if packages.is_empty() {
            println!("No kernel packages matched requested versions");
            return Ok(());
        }

        println!("Removing kernel packages: {}", packages.join(", "));
        let mut args = vec!["remove"];
        if yes {
            args.push("-y");
        }
        args.extend(packages.iter().map(|s| s.as_str()));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf remove kernels")?;
        self.history.log("kernel_remove", &packages)?;
        Ok(())
    }

    pub fn remove_old(&self, keep: usize, yes: bool) -> Result<()> {
        let mut installed = self.list_installed()?;
        if installed.len() <= keep {
            println!("Only {} kernel(s) installed, nothing to remove", installed.len());
            return Ok(());
        }
        let current = {
            let mut cmd = command("uname", &["-r"], false);
            run_capture_allow_fail(&mut cmd, "uname -r")?
                .map(|s| s.trim().to_string())
        };
        installed.sort();
        let mut keep_set = BTreeMap::new();
        for k in installed.iter().rev().take(keep) {
            keep_set.insert(k.clone(), true);
        }
        let targets: Vec<String> = installed
            .into_iter()
            .filter(|k| {
                if let Some(ref cur) = current {
                    if cur.contains(k) {
                        return false;
                    }
                }
                !keep_set.contains_key(k)
            })
            .collect();

        if targets.is_empty() {
            println!("No old kernels to remove");
            return Ok(());
        }
        self.remove(&targets, yes, true)
    }

    pub fn info(&self, version: Option<&str>) -> Result<()> {
        let target_version = if let Some(v) = version {
            v.to_string()
        } else {
            let mut cmd = command("uname", &["-r"], false);
            run_capture(&mut cmd, "uname -r")?.trim().to_string()
        };
        let query = format!("kernel*{target_version}*");
        let mut cmd = command("rpm", &["-qa", &query], false);
        let output = run_capture_allow_fail(&mut cmd, "rpm -qa kernel info")?;
        if output.is_none() {
            println!("Kernel version '{target_version}' not found");
            return Ok(());
        }
        let packages: Vec<String> = output
            .unwrap()
            .lines()
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty())
            .collect();
        if packages.is_empty() {
            println!("Kernel version '{target_version}' not found");
            return Ok(());
        }
        println!("Kernel version: {target_version}");
        println!("Packages:");
        for pkg in &packages {
            println!("  {pkg}");
        }
        Ok(())
    }

    pub fn cachyos_list(&self) -> Result<()> {
        let mut kernels = Vec::new();
        let mut cmd = command("dnf", &["list", "available", "kernel-cachyos*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list cachyos")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.contains("devel") || pkg.contains("headers") {
                        continue;
                    }
                    kernels.push(pkg.to_string());
                }
            }
        }
        let mut cmd = command("dnf", &["list", "available", "kernel-cachyos-lto*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list cachyos lto")? {
            for line in out.lines().skip(1) {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.contains("devel") || pkg.contains("headers") {
                        continue;
                    }
                    kernels.push(pkg.to_string());
                }
            }
        }
        if kernels.is_empty() {
            println!("No CachyOS kernels found (repositories might be disabled)");
        } else {
            println!("Available CachyOS kernels ({}):", kernels.len());
            for k in kernels {
                println!("  {k}");
            }
        }
        Ok(())
    }

    pub fn cachyos_enable(&self, repo_type: &str, yes: bool) -> Result<()> {
        let repo = match repo_type {
            "gcc" => "bieszczaders/kernel-cachyos",
            "lto" => "bieszczaders/kernel-cachyos-lto",
            other => anyhow::bail!("unknown CachyOS repo type: {other}"),
        };
        let mut args = vec!["copr", "enable"];
        if yes {
            args.push("-y");
        }
        args.push(repo);
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf copr enable cachyos")?;
        Ok(())
    }

    pub fn cachyos_check(&self) -> Result<()> {
        let mut cmd = command("dnf", &["repolist", "enabled"], false);
        let output = run_capture(&mut cmd, "dnf repolist enabled")?;
        let gcc = output.contains("kernel-cachyos");
        let lto = output.contains("kernel-cachyos-lto");
        println!("CachyOS GCC repo: {}", if gcc { "enabled" } else { "disabled" });
        println!("CachyOS LTO repo: {}", if lto { "enabled" } else { "disabled" });
        Ok(())
    }

    pub fn cachyos_install(&self, kernel_type: &str, build: &str, yes: bool) -> Result<()> {
        let target_repo = match build {
            "gcc" => "gcc",
            "lto" => "lto",
            other => anyhow::bail!("unknown build type: {other}"),
        };
        let mut repo_status = command("dnf", &["repolist", "enabled"], false);
        let output = run_capture(&mut repo_status, "dnf repolist")?;
        if !output.contains("kernel-cachyos") && target_repo == "gcc" {
            println!("Enabling CachyOS GCC repository...");
            self.cachyos_enable("gcc", yes)?;
        }
        if !output.contains("kernel-cachyos-lto") && target_repo == "lto" {
            println!("Enabling CachyOS LTO repository...");
            self.cachyos_enable("lto", yes)?;
        }

        let package = match (kernel_type, build) {
            ("lts", "lto") => "kernel-cachyos-lts-lto",
            ("lts", _) => "kernel-cachyos-lts",
            ("rt", "lto") => "kernel-cachyos-rt-lto",
            ("rt", _) => "kernel-cachyos-rt",
            ("server", "lto") => "kernel-cachyos-server-lto",
            ("server", _) => "kernel-cachyos-server",
            ("default", "lto") => "kernel-cachyos-lto",
            ("default", _) => "kernel-cachyos",
            (other, _) => anyhow::bail!("unknown kernel type: {other}"),
        };
        let packages = vec![package.to_string(), format!("{package}-devel-matched")];
        println!("Installing CachyOS kernel {package} ({build})");
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for p in &packages {
            args.push(p);
        }
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install cachyos kernel")?;
        self.history.log("cachyos_kernel_install", &packages)?;
        println!("Reboot to use the new CachyOS kernel");
        Ok(())
    }

    pub fn cachyos_check_cpu(&self) -> Result<()> {
        let mut support = vec![
            ("x86-64-v2", false),
            ("x86-64-v3", false),
            ("x86-64-v4", false),
        ];
        if let Ok(cpuinfo) = std::fs::read_to_string("/proc/cpuinfo") {
            let lower = cpuinfo.to_lowercase();
            let has_avx2 = lower.contains("avx2");
            let has_avx = lower.contains("avx");
            let has_avx512 = lower.contains("avx512");
            support.iter_mut().for_each(|(level, val)| match *level {
                "x86-64-v4" => *val = has_avx512,
                "x86-64-v3" => *val = has_avx2,
                "x86-64-v2" => *val = has_avx || has_avx2,
                _ => {}
            });
        }
        println!("CPU instruction set support:");
        for (lvl, ok) in support {
            println!("  {lvl}: {}", if ok { "supported" } else { "not detected" });
        }
        Ok(())
    }
}

