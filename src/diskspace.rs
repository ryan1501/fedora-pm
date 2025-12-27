use crate::runner::{command, run_capture};
use anyhow::Result;
use colored::Colorize;

pub struct DiskSpaceManager {
    use_sudo: bool,
}

impl DiskSpaceManager {
    pub fn new(use_sudo: bool) -> Self {
        Self { use_sudo }
    }

    pub fn top_packages(&self, limit: usize) -> Result<()> {
        println!("Top {} largest installed packages:", limit);
        println!("{}", "=".repeat(60));

        let mut cmd = command("rpm", &["-qa", "--queryformat", "%{SIZE} %{NAME}\n"], false);
        let output = run_capture(&mut cmd, "rpm query")?;

        let mut packages: Vec<(u64, String)> = output
            .lines()
            .filter_map(|line| {
                let parts: Vec<&str> = line.split_whitespace().collect();
                if parts.len() >= 2 {
                    if let Ok(size) = parts[0].parse::<u64>() {
                        return Some((size, parts[1].to_string()));
                    }
                }
                None
            })
            .collect();

        packages.sort_by(|a, b| b.0.cmp(&a.0));

        for (i, (size, name)) in packages.iter().take(limit).enumerate() {
            let size_mb = *size as f64 / 1024.0 / 1024.0;
            println!("{:3}. {:40} {:>10.2} MB", i + 1, name, size_mb);
        }

        Ok(())
    }

    pub fn total_size(&self) -> Result<()> {
        println!("Calculating total package size...");

        let mut cmd = command("rpm", &["-qa", "--queryformat", "%{SIZE}\n"], false);
        let output = run_capture(&mut cmd, "rpm query")?;

        let total: u64 = output
            .lines()
            .filter_map(|line| line.parse::<u64>().ok())
            .sum();

        let total_gb = total as f64 / 1024.0 / 1024.0 / 1024.0;
        let total_mb = total as f64 / 1024.0 / 1024.0;

        println!("{}", "=".repeat(60));
        println!("Total installed package size: {:.2} GB ({:.2} MB)", total_gb, total_mb);

        // Also show cache size
        self.cache_size()?;

        Ok(())
    }

    pub fn cache_size(&self) -> Result<()> {
        println!("\nDNF cache size:");

        let mut cmd = command("du", &["-sh", "/var/cache/dnf"], false);
        if let Ok(output) = run_capture(&mut cmd, "du") {
            let size = output.split_whitespace().next().unwrap_or("0");
            println!("  /var/cache/dnf: {}", size);
        }

        Ok(())
    }

    pub fn orphans(&self) -> Result<()> {
        println!("Finding orphaned packages...");
        println!("{}", "=".repeat(60));

        let mut cmd = command("dnf", &["repoquery", "--extras"], false);
        let output = run_capture(&mut cmd, "dnf repoquery")?;

        let orphans: Vec<&str> = output.lines().filter(|l| !l.is_empty()).collect();

        if orphans.is_empty() {
            println!("{}", "✓ No orphaned packages found".green());
        } else {
            println!("Found {} orphaned packages:", orphans.len());
            for orphan in &orphans {
                println!("  {}", orphan);
            }
            println!("\nTo remove orphaned packages, run:");
            println!("  {}", "fedora-pm clean-orphans".cyan());
        }

        Ok(())
    }

    pub fn remove_orphans(&self, yes: bool) -> Result<()> {
        println!("Removing orphaned packages...");

        let mut args = vec!["autoremove"];
        if yes {
            args.push("-y");
        }

        let mut cmd = command("dnf", &args, self.use_sudo);
        crate::runner::run_inherit(&mut cmd, "dnf autoremove")?;

        println!("{}", "✓ Orphaned packages removed".green());
        Ok(())
    }

    pub fn package_size(&self, package: &str) -> Result<()> {
        println!("Size information for: {}", package);
        println!("{}", "=".repeat(60));

        let mut cmd = command("rpm", &["-qi", package], false);
        let output = run_capture(&mut cmd, "rpm -qi")?;

        for line in output.lines() {
            if line.starts_with("Size") {
                println!("{}", line);
                break;
            }
        }

        // Also show dependencies size
        println!("\nDependencies:");
        let mut deps_cmd = command("dnf", &["repoquery", "--requires", package], false);
        if let Ok(deps_output) = run_capture(&mut deps_cmd, "dnf repoquery") {
            let deps: Vec<&str> = deps_output.lines().filter(|l| !l.is_empty()).collect();
            println!("  {} dependencies", deps.len());
        }

        Ok(())
    }

    pub fn analyze(&self) -> Result<()> {
        println!("{}", "Disk Space Analysis".bold().cyan());
        println!("{}", "=".repeat(60));

        // Total package size
        let mut cmd = command("rpm", &["-qa", "--queryformat", "%{SIZE}\n"], false);
        let output = run_capture(&mut cmd, "rpm query")?;
        let total: u64 = output
            .lines()
            .filter_map(|line| line.parse::<u64>().ok())
            .sum();
        let total_gb = total as f64 / 1024.0 / 1024.0 / 1024.0;

        println!("Total installed packages: {:.2} GB", total_gb);

        // Cache size
        let mut cache_cmd = command("du", &["-sh", "/var/cache/dnf"], false);
        if let Ok(cache_output) = run_capture(&mut cache_cmd, "du") {
            let size = cache_output.split_whitespace().next().unwrap_or("0");
            println!("DNF cache size:       {}", size);
        }

        // Root filesystem usage
        let mut df_cmd = command("df", &["-h", "/"], false);
        if let Ok(df_output) = run_capture(&mut df_cmd, "df") {
            let lines: Vec<&str> = df_output.lines().collect();
            if lines.len() > 1 {
                println!("\nRoot filesystem:");
                println!("{}", lines[1]);
            }
        }

        // Orphaned packages
        let mut orphan_cmd = command("dnf", &["repoquery", "--extras"], false);
        if let Ok(orphan_output) = run_capture(&mut orphan_cmd, "dnf repoquery") {
            let orphans: Vec<&str> = orphan_output.lines().filter(|l| !l.is_empty()).collect();
            println!("\nOrphaned packages:    {}", orphans.len());
        }

        println!("\n{}", "Recommendations:".bold());
        println!("  • Run 'fedora-pm clean' to clear cache");
        println!("  • Run 'fedora-pm clean-orphans' to remove orphaned packages");
        println!("  • Run 'fedora-pm size --top 20' to see largest packages");

        Ok(())
    }
}
