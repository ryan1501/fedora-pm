use crate::history::History;
use crate::runner::{command, run_inherit};
use anyhow::Result;

pub struct DownloadManager {
    use_sudo: bool,
    history: History,
}

impl DownloadManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn download(&self, packages: &[String]) -> Result<()> {
        if packages.is_empty() {
            anyhow::bail!("no packages specified");
        }

        println!("Downloading packages: {}", packages.join(", "));

        let mut args = vec!["download"];
        for pkg in packages {
            args.push(pkg.as_str());
        }

        let mut cmd = command("dnf", &args, false);
        run_inherit(&mut cmd, "dnf download")?;
        self.history.log("download", packages)?;
        println!("✓ Packages downloaded to current directory");
        Ok(())
    }

    pub fn download_to(&self, packages: &[String], dest_dir: &str) -> Result<()> {
        if packages.is_empty() {
            anyhow::bail!("no packages specified");
        }

        println!("Downloading packages to: {}", dest_dir);

        let mut args = vec!["download", "--destdir", dest_dir];
        for pkg in packages {
            args.push(pkg.as_str());
        }

        let mut cmd = command("dnf", &args, false);
        run_inherit(&mut cmd, "dnf download")?;
        self.history.log("download", packages)?;
        println!("✓ Packages downloaded to {}", dest_dir);
        Ok(())
    }

    pub fn install_offline(&self, rpm_files: &[String], yes: bool) -> Result<()> {
        if rpm_files.is_empty() {
            anyhow::bail!("no RPM files specified");
        }

        println!("Installing RPM files: {}", rpm_files.join(", "));

        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for rpm in rpm_files {
            args.push(rpm.as_str());
        }

        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install")?;
        self.history.log("install-offline", rpm_files)?;
        println!("✓ RPM files installed");
        Ok(())
    }

    pub fn download_with_deps(&self, packages: &[String], dest_dir: &str) -> Result<()> {
        if packages.is_empty() {
            anyhow::bail!("no packages specified");
        }

        println!("Downloading packages with dependencies to: {}", dest_dir);

        let mut args = vec!["download", "--resolve", "--destdir", dest_dir];
        for pkg in packages {
            args.push(pkg.as_str());
        }

        let mut cmd = command("dnf", &args, false);
        run_inherit(&mut cmd, "dnf download")?;
        self.history.log("download-with-deps", packages)?;
        println!("✓ Packages and dependencies downloaded to {}", dest_dir);
        Ok(())
    }
}
