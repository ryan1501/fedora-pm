use crate::history::History;
use crate::runner::{command, run_capture, run_inherit};
use anyhow::Result;

#[derive(Clone)]
pub struct PackageManager {
    use_sudo: bool,
    history: History,
}

impl PackageManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn install(&self, packages: &[String], yes: bool) -> Result<()> {
        if packages.is_empty() {
            anyhow::bail!("no packages specified");
        }
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for pkg in packages {
            args.push(pkg);
        }
        println!("Installing: {}", packages.join(", "));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install")?;
        self.history.log("install", packages)?;
        Ok(())
    }

    pub fn remove(&self, packages: &[String], yes: bool) -> Result<()> {
        if packages.is_empty() {
            anyhow::bail!("no packages specified");
        }
        let mut args = vec!["remove"];
        if yes {
            args.push("-y");
        }
        for pkg in packages {
            args.push(pkg);
        }
        println!("Removing: {}", packages.join(", "));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf remove")?;
        self.history.log("remove", packages)?;
        Ok(())
    }

    pub fn update(&self, packages: &[String], yes: bool) -> Result<()> {
        let mut args = vec!["update"];
        if yes {
            args.push("-y");
        }
        for pkg in packages {
            args.push(pkg);
        }
        println!(
            "Updating {}",
            if packages.is_empty() {
                "system".to_string()
            } else {
                packages.join(", ")
            }
        );
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf update")?;
        let items: Vec<String> = if packages.is_empty() {
            vec!["system".into()]
        } else {
            packages.to_vec()
        };
        self.history.log("update", &items)?;
        Ok(())
    }

    pub fn search(&self, query: &str) -> Result<()> {
        let mut cmd = command("dnf", &["search", query], false);
        let output = run_capture(&mut cmd, "dnf search")?;
        println!("{output}");
        Ok(())
    }

    pub fn info(&self, package: &str) -> Result<()> {
        let mut rpm_cmd = command("rpm", &["-qi", package], false);
        if let Ok(info) = run_capture(&mut rpm_cmd, "rpm -qi") {
            println!("{info}");
            return Ok(());
        }

        let mut dnf_cmd = command("dnf", &["info", package], false);
        if let Ok(info) = run_capture(&mut dnf_cmd, "dnf info") {
            println!("{info}");
            return Ok(());
        }

        anyhow::bail!("package {package} not found");
    }

    pub fn list_installed(&self, pattern: Option<&str>) -> Result<()> {
        let mut args = vec!["-qa"];
        if let Some(p) = pattern {
            args.push(p);
        }
        let mut cmd = command("rpm", &args, false);
        let output = run_capture(&mut cmd, "rpm -qa")?;
        let packages: Vec<&str> = output.lines().filter(|l| !l.is_empty()).collect();
        println!("Installed packages ({})", packages.len());
        for pkg in packages.iter().take(100) {
            println!("  {pkg}");
        }
        Ok(())
    }

    pub fn list_available(&self, pattern: Option<&str>) -> Result<()> {
        let mut args = vec!["list", "available"];
        if let Some(p) = pattern {
            args.push(p);
        }
        let mut cmd = command("dnf", &args, false);
        let output = run_capture(&mut cmd, "dnf list available")?;
        let packages: Vec<&str> = output
            .lines()
            .skip(1)
            .filter_map(|line| line.split_whitespace().next())
            .collect();
        println!("Available packages ({})", packages.len());
        for pkg in packages.iter().take(100) {
            println!("  {pkg}");
        }
        Ok(())
    }

    pub fn clean(&self, cache: bool, metadata: bool) -> Result<()> {
        println!("Cleaning dnf cache...");
        if cache {
            let mut cmd = command("dnf", &["clean", "packages"], self.use_sudo);
            run_inherit(&mut cmd, "dnf clean packages")?;
        }
        if metadata {
            let mut cmd = command("dnf", &["clean", "metadata"], self.use_sudo);
            run_inherit(&mut cmd, "dnf clean metadata")?;
        }
        if cache && metadata {
            let mut cmd = command("dnf", &["clean", "all"], self.use_sudo);
            run_inherit(&mut cmd, "dnf clean all")?;
        }
        self.history.log("clean", &["dnf".into()])?;
        println!("Clean completed");
        Ok(())
    }
}

