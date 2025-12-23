use crate::history::History;
use crate::runner::{command, run_inherit};
use anyhow::Result;

#[derive(Clone)]
pub struct GamingManager {
    use_sudo: bool,
    history: History,
}

impl GamingManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn install_meta(&self, yes: bool) -> Result<()> {
        // The meta package is expected to be provided by the repo/spec in this project.
        let package = "fedora-gaming-meta";
        println!("Installing gaming meta package ({package})");
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        args.push(package);
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install gaming meta")?;
        self.history.log("gaming_install", &[package.to_string()])?;
        Ok(())
    }
}

