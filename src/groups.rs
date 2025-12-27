use crate::history::History;
use crate::runner::{command, run_capture, run_inherit};
use anyhow::Result;

pub struct GroupManager {
    use_sudo: bool,
    history: History,
}

impl GroupManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn list(&self) -> Result<()> {
        println!("Available package groups:");
        let mut cmd = command("dnf", &["group", "list"], false);
        let output = run_capture(&mut cmd, "dnf group list")?;
        println!("{}", output);
        Ok(())
    }

    pub fn info(&self, group: &str) -> Result<()> {
        println!("Group information for: {}", group);
        let mut cmd = command("dnf", &["group", "info", group], false);
        let output = run_capture(&mut cmd, "dnf group info")?;
        println!("{}", output);
        Ok(())
    }

    pub fn install(&self, group: &str, yes: bool) -> Result<()> {
        let mut args = vec!["group", "install"];
        if yes {
            args.push("-y");
        }
        args.push(group);

        println!("Installing group: {}", group);
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf group install")?;
        self.history.log("group-install", &[group.to_string()])?;
        Ok(())
    }

    pub fn remove(&self, group: &str, yes: bool) -> Result<()> {
        let mut args = vec!["group", "remove"];
        if yes {
            args.push("-y");
        }
        args.push(group);

        println!("Removing group: {}", group);
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf group remove")?;
        self.history.log("group-remove", &[group.to_string()])?;
        Ok(())
    }
}
