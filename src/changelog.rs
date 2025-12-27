use crate::runner::{command, run_capture};
use anyhow::Result;

pub struct ChangelogManager {
    use_sudo: bool,
}

impl ChangelogManager {
    pub fn new(use_sudo: bool) -> Self {
        Self { use_sudo }
    }

    pub fn show(&self, package: &str) -> Result<()> {
        println!("Changelog for: {}", package);
        println!("{}", "=".repeat(60));

        // Try rpm first for installed packages
        let mut rpm_cmd = command("rpm", &["-q", "--changelog", package], false);
        if let Ok(output) = run_capture(&mut rpm_cmd, "rpm changelog") {
            if !output.is_empty() {
                println!("{}", output);
                return Ok(());
            }
        }

        // Try dnf for available packages
        let mut dnf_cmd = command("dnf", &["changelog", package], false);
        match run_capture(&mut dnf_cmd, "dnf changelog") {
            Ok(output) => {
                println!("{}", output);
                Ok(())
            }
            Err(_) => {
                anyhow::bail!("Could not retrieve changelog for {}", package)
            }
        }
    }

    pub fn show_recent(&self, package: &str, limit: usize) -> Result<()> {
        println!("Recent changelog entries for: {} (last {})", package, limit);
        println!("{}", "=".repeat(60));

        let mut cmd = command("rpm", &["-q", "--changelog", package], false);
        let output = run_capture(&mut cmd, "rpm changelog")?;

        let lines: Vec<&str> = output.lines().collect();
        let mut count = 0;
        let mut in_entry = false;

        for line in lines {
            if line.starts_with('*') {
                if count >= limit {
                    break;
                }
                count += 1;
                in_entry = true;
            }

            if in_entry {
                println!("{}", line);
            }
        }

        Ok(())
    }

    pub fn whatsnew(&self) -> Result<()> {
        println!("Checking for updates and their changelogs...");
        println!("{}", "=".repeat(60));

        // Get list of available updates
        let mut check_cmd = command("dnf", &["check-update"], false);
        let output = match run_capture(&mut check_cmd, "dnf check-update") {
            Ok(out) => out,
            Err(_) => {
                // check-update returns non-zero when updates are available
                let mut cmd = command("dnf", &["check-update"], false);
                cmd.output()
                    .map(|o| String::from_utf8_lossy(&o.stdout).to_string())
                    .unwrap_or_default()
            }
        };

        let packages: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with("Last"))
            .filter_map(|l| l.split_whitespace().next())
            .collect();

        if packages.is_empty() {
            println!("No updates available");
            return Ok(());
        }

        println!("Updates available for {} packages:\n", packages.len());

        for (i, pkg) in packages.iter().take(10).enumerate() {
            // Extract package name without version
            let pkg_name = pkg.split('.').next().unwrap_or(pkg);

            println!("{}. {}", i + 1, pkg);
            println!("{}", "-".repeat(60));

            // Show recent changelog
            let mut cmd = command("rpm", &["-q", "--changelog", pkg_name], false);
            if let Ok(changelog) = run_capture(&mut cmd, "rpm changelog") {
                let lines: Vec<&str> = changelog.lines().take(5).collect();
                for line in lines {
                    println!("  {}", line);
                }
            }
            println!();
        }

        if packages.len() > 10 {
            println!("... and {} more packages", packages.len() - 10);
        }

        Ok(())
    }

    pub fn compare(&self, package: &str, version1: &str, version2: &str) -> Result<()> {
        println!(
            "Comparing changelogs for {} between {} and {}",
            package, version1, version2
        );
        println!("{}", "=".repeat(60));

        // This is a simplified version - in practice, you'd need to parse versions
        println!("Note: This shows the full changelog. Manual comparison needed.");
        self.show(package)?;

        Ok(())
    }
}
