use crate::history::History;
use crate::runner::{command, run_capture, run_capture_allow_fail, run_inherit};
use anyhow::Result;

pub struct CachyOSManager {
    use_sudo: bool,
    history: History,
}

impl CachyOSManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn enable_repository(&self, repo_type: &str, yes: bool) -> Result<()> {
        let repo = match repo_type {
            "gcc" => "bieszczaders/kernel-cachyos",
            "lto" => "bieszczaders/kernel-cachyos-lto",
            "bore" => "bieszczaders/kernel-cachyos-bore",
            other => anyhow::bail!("unknown CachyOS repo type: {other}. Valid types: gcc, lto, bore"),
        };

        println!("Enabling CachyOS {} repository...", repo_type);
        let mut args = vec!["copr", "enable"];
        if yes {
            args.push("-y");
        }
        args.push(repo);
        
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf copr enable cachyos")?;
        self.history.log("cachyos_repo_enable", &[repo_type.to_string()])?;
        println!("✓ CachyOS {} repository enabled", repo_type);
        Ok(())
    }

    pub fn disable_repository(&self, repo_type: &str, yes: bool) -> Result<()> {
        let repo = match repo_type {
            "gcc" => "bieszczaders/kernel-cachyos",
            "lto" => "bieszczaders/kernel-cachyos-lto",
            "bore" => "bieszczaders/kernel-cachyos-bore",
            other => anyhow::bail!("unknown CachyOS repo type: {other}. Valid types: gcc, lto, bore"),
        };

        println!("Disabling CachyOS {} repository...", repo_type);
        let mut args = vec!["copr", "disable"];
        if yes {
            args.push("-y");
        }
        args.push(repo);
        
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf copr disable cachyos")?;
        self.history.log("cachyos_repo_disable", &[repo_type.to_string()])?;
        println!("✓ CachyOS {} repository disabled", repo_type);
        Ok(())
    }

    pub fn list_kernels(&self) -> Result<()> {
        println!("=== Available CachyOS Kernels ===");
        
        // Get all CachyOS kernels
        let mut kernels: std::collections::HashMap<String, Vec<String>> = std::collections::HashMap::new();
        
        // GCC kernels
        let mut cmd = command("dnf", &["list", "available", "kernel-cachyos*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "dnf list cachyos kernels")? {
            let output_lines: Vec<&str> = out.lines().skip(1).collect();
            for line in output_lines {
                if let Some(pkg) = line.split_whitespace().next() {
                    if pkg.contains("kernel-cachyos") && 
                       !pkg.contains("devel") && 
                       !pkg.contains("headers") && 
                       !pkg.contains("modules") {
                        if pkg.contains("lto") {
                            kernels.entry("LTO".to_string()).or_insert_with(Vec::new).push(pkg.to_string());
                        } else if pkg.contains("bore") {
                            kernels.entry("BORE".to_string()).or_insert_with(Vec::new).push(pkg.to_string());
                        } else {
                            kernels.entry("GCC".to_string()).or_insert_with(Vec::new).push(pkg.to_string());
                        }
                    }
                }
            }
        }

        if kernels.is_empty() {
            println!("No CachyOS kernels found. Enable CachyOS repositories first:");
            println!("  fedora-pm cachyos enable gcc");
            println!("  fedora-pm cachyos enable lto");
            println!("  fedora-pm cachyos enable bore");
            return Ok(());
        }

        for (category, kernel_list) in kernels {
            if !kernel_list.is_empty() {
                println!("\n{} kernels:", category);
                for kernel in &kernel_list {
                    if let Some(kernel_info) = self.get_kernel_info(kernel) {
                        println!("  {} - {}", kernel, kernel_info);
                    } else {
                        println!("  {}", kernel);
                    }
                }
            }
        }

        Ok(())
    }

    pub fn install_kernel(&self, kernel_type: &str, variant: Option<&str>, yes: bool) -> Result<()> {
        let build_type = variant.unwrap_or("gcc");
        
        let package = match (kernel_type, build_type) {
            ("lts", "lto") => "kernel-cachyos-lts-lto",
            ("lts", "bore") => "kernel-cachyos-lts-bore",
            ("lts", "gcc") | ("lts", _) => "kernel-cachyos-lts",
            ("rt", "lto") => "kernel-cachyos-rt-lto",
            ("rt", "bore") => "kernel-cachyos-rt-bore",
            ("rt", "gcc") | ("rt", _) => "kernel-cachyos-rt",
            ("server", "lto") => "kernel-cachyos-server-lto",
            ("server", "bore") => "kernel-cachyos-server-bore",
            ("server", "gcc") | ("server", _) => "kernel-cachyos-server",
            ("default", "lto") => "kernel-cachyos-lto",
            ("default", "bore") => "kernel-cachyos-bore",
            ("default", "gcc") | ("default", _) => "kernel-cachyos",
            (other, _) => anyhow::bail!("unknown kernel type: {other}. Valid types: lts, rt, server, default"),
        };

        // Check if repository is enabled
        self.ensure_repository_enabled(build_type, yes)?;

        let packages = vec![
            package.to_string(),
            format!("{package}-devel-matched"),
        ];

        println!("Installing CachyOS {} kernel ({})...", kernel_type, build_type);
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for pkg in &packages {
            args.push(pkg);
        }

        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install cachyos kernel")?;
        self.history.log("cachyos_kernel_install", &packages)?;
        
        println!("✓ CachyOS {} kernel ({}) installed successfully", kernel_type, build_type);
        println!("Reboot to use the new kernel");
        Ok(())
    }

    pub fn check_cpu_features(&self) -> Result<()> {
        println!("=== CPU Feature Detection for CachyOS Kernels ===");
        
        let mut features = vec![
            ("x86-64-v2", "Basic 64-bit", false),
            ("x86-64-v3", "Modern (AVX2)", false),
            ("x86-64-v4", "High-end (AVX512)", false),
        ];

        if let Ok(cpuinfo) = std::fs::read_to_string("/proc/cpuinfo") {
            let lower = cpuinfo.to_lowercase();
            let has_avx512 = lower.contains("avx512") && lower.contains("avx512f");
            let has_avx2 = lower.contains("avx2");
            let _has_avx = lower.contains("avx");
            let has_sse42 = lower.contains("sse4_2");
            
            features[0].2 = has_sse42; // x86-64-v2 requires SSE4.2
            features[1].2 = has_avx2;   // x86-64-v3 requires AVX2
            features[2].2 = has_avx512; // x86-64-v4 requires AVX512
        }

        println!("CPU microarchitecture support:");
        for (level, desc, supported) in &features {
            let status = if *supported { "✓ Supported" } else { "✗ Not supported" };
            println!("  {} ({}): {}", level, desc, status);
        }

        let recommended = if features[2].2 {
            "Use x86-64-v4 optimized kernels for best performance"
        } else if features[1].2 {
            "Use x86-64-v3 optimized kernels (LTO recommended)"
        } else if features[0].2 {
            "Use standard GCC kernels or x86-64-v2 optimized kernels"
        } else {
            "Use standard kernels - CPU may not support optimizations"
        };

        println!("\nRecommendation: {}", recommended);

        // Check if BORE scheduler is supported
        println!("\nBORE Scheduler Support:");
        println!("  ✓ BORE (Burst-Oriented Response Enhancer) available in CachyOS-BORE kernels");
        println!("  ✓ Improves desktop responsiveness and gaming performance");
        
        Ok(())
    }

    pub fn get_status(&self) -> Result<()> {
        println!("=== CachyOS Status ===");
        
        // Check repository status
        let mut cmd = command("dnf", &["repolist", "enabled"], false);
        let output = run_capture_allow_fail(&mut cmd, "dnf repolist")?.unwrap_or_default();
        
        let gcc_enabled = output.contains("kernel-cachyos") && !output.contains("kernel-cachyos-lto");
        let lto_enabled = output.contains("kernel-cachyos-lto");
        let bore_enabled = output.contains("kernel-cachyos-bore");
        
        println!("Repository Status:");
        println!("  GCC:     {}", if gcc_enabled { "✓ Enabled" } else { "✗ Disabled" });
        println!("  LTO:     {}", if lto_enabled { "✓ Enabled" } else { "✗ Disabled" });
        println!("  BORE:    {}", if bore_enabled { "✓ Enabled" } else { "✗ Disabled" });
        
        // Check installed kernels
        let mut cmd = command("rpm", &["-qa", "kernel-cachyos*"], false);
        if let Some(out) = run_capture_allow_fail(&mut cmd, "rpm -qa cachyos")? {
            let kernels: Vec<&str> = out.lines()
                .filter(|l| l.starts_with("kernel-cachyos"))
                .filter(|l| !l.contains("devel") && !l.contains("headers"))
                .collect();
            
            if kernels.is_empty() {
                println!("\nInstalled Kernels: None");
            } else {
                println!("\nInstalled Kernels ({}):", kernels.len());
                for kernel in &kernels {
                    println!("  {}", kernel);
                }
            }
        }
        
        // Check current kernel
        let mut cmd = command("uname", &["-r"], false);
        if let Some(current) = run_capture_allow_fail(&mut cmd, "uname -r")? {
            let current_trim = current.trim();
            if current_trim.contains("cachyos") {
                println!("\nCurrent Kernel: {}", current_trim);
                println!("  ✓ Running CachyOS kernel");
            } else {
                println!("\nCurrent Kernel: {}", current_trim);
                println!("  ✗ Not running CachyOS kernel");
            }
        }
        
        Ok(())
    }

    fn ensure_repository_enabled(&self, build_type: &str, yes: bool) -> Result<()> {
        let mut cmd = command("dnf", &["repolist", "enabled"], false);
        let output = run_capture(&mut cmd, "dnf repolist")?;
        
        let repo_needed = match build_type {
            "lto" => "kernel-cachyos-lto",
            "bore" => "kernel-cachyos-bore",
            "gcc" | _ => "kernel-cachyos",
        };
        
        if !output.contains(repo_needed) {
            println!("CachyOS {} repository not enabled. Enabling...", build_type);
            self.enable_repository(build_type, yes)?;
        }
        
        Ok(())
    }

    fn get_kernel_info(&self, package_name: &str) -> Option<String> {
        if package_name.contains("lts") {
            Some("Long Term Support".to_string())
        } else if package_name.contains("rt") {
            Some("Real-time (PREEMPT_RT)".to_string())
        } else if package_name.contains("server") {
            Some("Server Optimized".to_string())
        } else if package_name.contains("lto") {
            Some("Link-Time Optimized".to_string())
        } else if package_name.contains("bore") {
            Some("BORE Scheduler".to_string())
        } else {
            None
        }
    }
}