use crate::history::History;
use crate::runner::{command, run_capture, run_inherit};
use anyhow::Result;
use colored::Colorize;

pub struct SecurityManager {
    use_sudo: bool,
    history: History,
}

impl SecurityManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn check(&self) -> Result<()> {
        println!("{}", "Checking for security updates...".bold().cyan());
        let mut cmd = command("dnf", &["updateinfo", "list", "security"], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;

        let updates: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with("Last"))
            .collect();

        if updates.is_empty() {
            println!("{}", "✓ No security updates available".green());
        } else {
            println!(
                "{}",
                format!("⚠ {} security updates available:", updates.len())
                    .yellow()
                    .bold()
            );
            for update in &updates {
                println!("  {}", update);
            }
        }

        Ok(())
    }

    pub fn list(&self, severity: Option<&str>) -> Result<()> {
        let mut args = vec!["updateinfo", "list"];

        if let Some(sev) = severity {
            args.push(sev);
        } else {
            args.push("security");
        }

        println!("Security updates:");
        let mut cmd = command("dnf", &args, false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;
        println!("{}", output);
        Ok(())
    }

    pub fn update(&self, yes: bool) -> Result<()> {
        println!("Installing security updates...");

        let mut args = vec!["update", "--security"];
        if yes {
            args.push("-y");
        }

        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf update --security")?;
        self.history.log("security-update", &["all".to_string()])?;
        println!("✓ Security updates installed");
        Ok(())
    }

    pub fn info(&self, advisory_id: &str) -> Result<()> {
        println!("Security advisory information:");
        let mut cmd = command("dnf", &["updateinfo", "info", advisory_id], false);
        let output = run_capture(&mut cmd, "dnf updateinfo info")?;
        println!("{}", output);
        Ok(())
    }

    pub fn cve_check(&self, cve_id: &str) -> Result<()> {
        println!("Checking for CVE: {}", cve_id);
        let mut cmd = command("dnf", &["updateinfo", "list", "--cve", cve_id], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;

        if output.trim().is_empty() {
            println!("No updates found for CVE: {}", cve_id);
        } else {
            println!("{}", output);
        }
        Ok(())
    }

    pub fn audit(&self) -> Result<()> {
        println!("{}", "Running security audit...".bold().cyan());
        println!();

        // Check for security updates
        print!("Security updates: ");
        let mut cmd = command("dnf", &["updateinfo", "list", "security"], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;
        let updates: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with("Last"))
            .collect();

        if updates.is_empty() {
            println!("{}", "✓ None".green());
        } else {
            println!("{}", format!("⚠ {} available", updates.len()).yellow());
        }

        // Check for critical updates
        print!("Critical updates: ");
        let mut cmd = command("dnf", &["updateinfo", "list", "critical"], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;
        let critical: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with("Last"))
            .collect();

        if critical.is_empty() {
            println!("{}", "✓ None".green());
        } else {
            println!("{}", format!("⚠ {} available", critical.len()).red().bold());
        }

        // Check for important updates
        print!("Important updates: ");
        let mut cmd = command("dnf", &["updateinfo", "list", "important"], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;
        let important: Vec<&str> = output
            .lines()
            .filter(|l| !l.is_empty() && !l.starts_with("Last"))
            .collect();

        if important.is_empty() {
            println!("{}", "✓ None".green());
        } else {
            println!("{}", format!("⚠ {} available", important.len()).yellow());
        }

        println!();
        if !critical.is_empty() || !updates.is_empty() {
            println!("Run 'fedora-pm security update' to install security updates");
        }

        Ok(())
    }
}
