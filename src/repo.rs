use crate::history::History;
use crate::runner::{command, run_capture, run_inherit};
use anyhow::Result;

pub struct RepoManager {
    use_sudo: bool,
    history: History,
}

impl RepoManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn list(&self, enabled_only: bool) -> Result<()> {
        let args = if enabled_only {
            vec!["repolist", "enabled"]
        } else {
            vec!["repolist", "all"]
        };

        println!("Repositories:");
        let mut cmd = command("dnf", &args, false);
        let output = run_capture(&mut cmd, "dnf repolist")?;
        println!("{}", output);
        Ok(())
    }

    pub fn enable(&self, repo_id: &str) -> Result<()> {
        println!("Enabling repository: {}", repo_id);
        let mut cmd = command("dnf", &["config-manager", "--set-enabled", repo_id], self.use_sudo);
        run_inherit(&mut cmd, "dnf config-manager")?;
        self.history.log("repo-enable", &[repo_id.to_string()])?;
        println!("✓ Repository enabled: {}", repo_id);
        Ok(())
    }

    pub fn disable(&self, repo_id: &str) -> Result<()> {
        println!("Disabling repository: {}", repo_id);
        let mut cmd = command("dnf", &["config-manager", "--set-disabled", repo_id], self.use_sudo);
        run_inherit(&mut cmd, "dnf config-manager")?;
        self.history.log("repo-disable", &[repo_id.to_string()])?;
        println!("✓ Repository disabled: {}", repo_id);
        Ok(())
    }

    pub fn add(&self, name: &str, url: &str) -> Result<()> {
        println!("Adding repository: {} ({})", name, url);
        let mut cmd = command("dnf", &["config-manager", "--add-repo", url], self.use_sudo);
        run_inherit(&mut cmd, "dnf config-manager")?;
        self.history.log("repo-add", &[name.to_string(), url.to_string()])?;
        println!("✓ Repository added: {}", name);
        Ok(())
    }

    pub fn remove(&self, repo_id: &str) -> Result<()> {
        println!("Removing repository: {}", repo_id);

        // First disable it
        let mut disable_cmd = command("dnf", &["config-manager", "--set-disabled", repo_id], self.use_sudo);
        run_inherit(&mut disable_cmd, "dnf config-manager")?;

        // Remove the repo file
        let repo_file = format!("/etc/yum.repos.d/{}.repo", repo_id);
        let mut rm_cmd = command("rm", &["-f", &repo_file], self.use_sudo);
        run_inherit(&mut rm_cmd, "rm repo file")?;

        self.history.log("repo-remove", &[repo_id.to_string()])?;
        println!("✓ Repository removed: {}", repo_id);
        Ok(())
    }

    pub fn info(&self, repo_id: &str) -> Result<()> {
        println!("Repository information for: {}", repo_id);
        let mut cmd = command("dnf", &["repoinfo", repo_id], false);
        let output = run_capture(&mut cmd, "dnf repoinfo")?;
        println!("{}", output);
        Ok(())
    }

    pub fn refresh(&self) -> Result<()> {
        println!("Refreshing repository metadata...");
        let mut cmd = command("dnf", &["clean", "metadata"], self.use_sudo);
        run_inherit(&mut cmd, "dnf clean metadata")?;

        let mut cmd = command("dnf", &["makecache"], self.use_sudo);
        run_inherit(&mut cmd, "dnf makecache")?;

        println!("✓ Repository metadata refreshed");
        Ok(())
    }
}
