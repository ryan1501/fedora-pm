use crate::runner::{command, run_capture, run_capture_allow_fail};
use anyhow::Result;
use colored::Colorize;
use std::fs;

pub struct DoctorManager {
    use_sudo: bool,
}

impl DoctorManager {
    pub fn new(use_sudo: bool) -> Self {
        Self { use_sudo }
    }

    pub fn check(&self) -> Result<()> {
        println!("{}", "Running system health check...".bold().cyan());
        println!();

        let mut issues = 0;

        // Check for broken dependencies
        issues += self.check_broken_dependencies()?;

        // Check for orphaned packages
        issues += self.check_orphaned_packages()?;

        // Check disk space
        issues += self.check_disk_space()?;

        // Check repository status
        issues += self.check_repositories()?;

        // Check for duplicate packages
        issues += self.check_duplicates()?;

        // Check for security updates
        issues += self.check_security_updates()?;

        println!();
        if issues == 0 {
            println!("{}", "✓ All checks passed! System is healthy.".green().bold());
        } else {
            println!(
                "{}",
                format!("⚠ Found {} issue(s) that may need attention.", issues)
                    .yellow()
                    .bold()
            );
        }

        Ok(())
    }

    fn check_broken_dependencies(&self) -> Result<usize> {
        print!("Checking for broken dependencies... ");
        let mut cmd = command("dnf", &["check"], false);
        match run_capture_allow_fail(&mut cmd, "dnf check")? {
            Some(output) => {
                if output.contains("Error") || output.contains("problem") {
                    println!("{}", "✗ ISSUES FOUND".red());
                    println!("{}", output);
                    Ok(1)
                } else {
                    println!("{}", "✓ OK".green());
                    Ok(0)
                }
            }
            None => {
                println!("{}", "✗ ISSUES FOUND".red());
                Ok(1)
            }
        }
    }

    fn check_orphaned_packages(&self) -> Result<usize> {
        print!("Checking for orphaned packages... ");
        let mut cmd = command("dnf", &["repoquery", "--extras"], false);
        match run_capture(&mut cmd, "dnf repoquery") {
            Ok(output) => {
                let orphans: Vec<&str> = output.lines().filter(|l| !l.is_empty()).collect();
                if orphans.is_empty() {
                    println!("{}", "✓ OK".green());
                    Ok(0)
                } else {
                    println!("{}", format!("⚠ Found {} orphaned packages", orphans.len()).yellow());
                    println!("  Run 'dnf autoremove' to clean them up");
                    Ok(1)
                }
            }
            Err(_) => {
                println!("{}", "✓ OK".green());
                Ok(0)
            }
        }
    }

    fn check_disk_space(&self) -> Result<usize> {
        print!("Checking disk space... ");

        // Check /var/cache/dnf
        let cache_path = "/var/cache/dnf";
        if let Ok(metadata) = fs::metadata(cache_path) {
            // Estimate cache size (simplified)
            let mut cmd = command("du", &["-sh", cache_path], false);
            if let Ok(output) = run_capture(&mut cmd, "du") {
                let size = output.split_whitespace().next().unwrap_or("0");
                println!("{}", format!("Cache size: {}", size).cyan());
            }
        }

        // Check root filesystem
        let mut cmd = command("df", &["-h", "/"], false);
        if let Ok(output) = run_capture(&mut cmd, "df") {
            let lines: Vec<&str> = output.lines().collect();
            if lines.len() > 1 {
                let parts: Vec<&str> = lines[1].split_whitespace().collect();
                if parts.len() > 4 {
                    let usage = parts[4].trim_end_matches('%');
                    if let Ok(usage_num) = usage.parse::<u32>() {
                        if usage_num > 90 {
                            println!("{}", format!("✗ Disk usage at {}%", usage_num).red());
                            return Ok(1);
                        } else if usage_num > 80 {
                            println!("{}", format!("⚠ Disk usage at {}%", usage_num).yellow());
                            return Ok(1);
                        }
                    }
                }
            }
        }

        println!("{}", "✓ OK".green());
        Ok(0)
    }

    fn check_repositories(&self) -> Result<usize> {
        print!("Checking repository status... ");
        let mut cmd = command("dnf", &["repolist"], false);
        match run_capture(&mut cmd, "dnf repolist") {
            Ok(output) => {
                if output.contains("repolist: 0") {
                    println!("{}", "✗ No repositories enabled".red());
                    Ok(1)
                } else {
                    println!("{}", "✓ OK".green());
                    Ok(0)
                }
            }
            Err(_) => {
                println!("{}", "✗ Failed to check repositories".red());
                Ok(1)
            }
        }
    }

    fn check_duplicates(&self) -> Result<usize> {
        print!("Checking for duplicate packages... ");
        let mut cmd = command("dnf", &["repoquery", "--duplicates"], false);
        match run_capture(&mut cmd, "dnf repoquery") {
            Ok(output) => {
                let duplicates: Vec<&str> = output.lines().filter(|l| !l.is_empty()).collect();
                if duplicates.is_empty() {
                    println!("{}", "✓ OK".green());
                    Ok(0)
                } else {
                    println!("{}", format!("⚠ Found {} duplicates", duplicates.len()).yellow());
                    for dup in duplicates.iter().take(5) {
                        println!("    {}", dup);
                    }
                    Ok(1)
                }
            }
            Err(_) => {
                println!("{}", "✓ OK".green());
                Ok(0)
            }
        }
    }

    fn check_security_updates(&self) -> Result<usize> {
        print!("Checking for security updates... ");
        let mut cmd = command("dnf", &["updateinfo", "list", "security"], false);
        match run_capture(&mut cmd, "dnf updateinfo") {
            Ok(output) => {
                let updates: Vec<&str> = output.lines().filter(|l| !l.is_empty() && !l.starts_with("Last")).collect();
                if updates.is_empty() {
                    println!("{}", "✓ OK".green());
                    Ok(0)
                } else {
                    println!("{}", format!("⚠ {} security updates available", updates.len()).yellow());
                    println!("  Run 'fedora-pm security update' to install them");
                    Ok(1)
                }
            }
            Err(_) => {
                println!("{}", "✓ OK".green());
                Ok(0)
            }
        }
    }
}
