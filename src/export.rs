use crate::runner::{command, run_capture};
use anyhow::Result;
use std::fs;
use std::path::Path;

pub struct ExportManager {
    use_sudo: bool,
}

impl ExportManager {
    pub fn new(use_sudo: bool) -> Self {
        Self { use_sudo }
    }

    pub fn export(&self, output_file: &str) -> Result<()> {
        println!("Exporting installed packages to: {}", output_file);

        // Get list of explicitly installed packages
        let mut cmd = command("dnf", &["repoquery", "--userinstalled"], false);
        let output = run_capture(&mut cmd, "dnf repoquery")?;

        let packages: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty())
            .collect();

        println!("Found {} user-installed packages", packages.len());

        // Write to file
        fs::write(output_file, packages.join("\n"))?;
        println!("✓ Package list exported successfully!");

        Ok(())
    }

    pub fn import(&self, input_file: &str, yes: bool) -> Result<()> {
        if !Path::new(input_file).exists() {
            anyhow::bail!("File not found: {}", input_file);
        }

        println!("Importing packages from: {}", input_file);
        let content = fs::read_to_string(input_file)?;
        let packages: Vec<&str> = content
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with('#'))
            .collect();

        println!("Found {} packages to install", packages.len());

        if !yes {
            println!("\nPackages to install:");
            for pkg in &packages {
                println!("  {}", pkg);
            }
            println!("\nProceed with installation? (y/N)");
            let mut input = String::new();
            std::io::stdin().read_line(&mut input)?;
            if !input.trim().eq_ignore_ascii_case("y") {
                println!("Installation cancelled");
                return Ok(());
            }
        }

        // Install packages
        let mut args = vec!["install", "-y"];
        for pkg in &packages {
            args.push(pkg);
        }

        let mut cmd = command("dnf", &args, self.use_sudo);
        crate::runner::run_inherit(&mut cmd, "dnf install")?;

        println!("✓ Packages imported successfully!");
        Ok(())
    }

    pub fn export_with_flatpak(&self, output_file: &str) -> Result<()> {
        println!("Exporting packages and Flatpaks to: {}", output_file);

        // Get DNF packages
        let mut cmd = command("dnf", &["repoquery", "--userinstalled"], false);
        let dnf_output = run_capture(&mut cmd, "dnf repoquery")?;

        let mut content = String::new();
        content.push_str("# DNF Packages\n");
        content.push_str(&dnf_output);

        // Get Flatpaks if available
        let mut flatpak_cmd = command("flatpak", &["list", "--app", "--columns=application"], false);
        if let Ok(flatpak_output) = run_capture(&mut flatpak_cmd, "flatpak list") {
            content.push_str("\n# Flatpak Applications\n");
            content.push_str("# flatpak:\n");
            for line in flatpak_output.lines() {
                if !line.is_empty() {
                    content.push_str(&format!("# flatpak:{}\n", line));
                }
            }
        }

        fs::write(output_file, content)?;
        println!("✓ Package list exported successfully!");

        Ok(())
    }
}
