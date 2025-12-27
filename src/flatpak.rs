use crate::history::History;
use crate::runner::{command, run_capture, run_inherit};
use anyhow::Result;

pub struct FlatpakManager {
    use_sudo: bool,
    history: History,
}

impl FlatpakManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    fn check_flatpak_installed(&self) -> Result<bool> {
        let mut cmd = command("which", &["flatpak"], false);
        Ok(run_capture(&mut cmd, "which flatpak").is_ok())
    }

    fn ensure_flatpak(&self) -> Result<()> {
        if !self.check_flatpak_installed()? {
            println!("Flatpak is not installed. Installing...");
            let mut cmd = command("dnf", &["install", "-y", "flatpak"], self.use_sudo);
            run_inherit(&mut cmd, "dnf install flatpak")?;
        }
        Ok(())
    }

    pub fn search(&self, query: &str) -> Result<()> {
        self.ensure_flatpak()?;
        println!("Searching Flatpak for: {}", query);
        let mut cmd = command("flatpak", &["search", query], false);
        let output = run_capture(&mut cmd, "flatpak search")?;
        println!("{}", output);
        Ok(())
    }

    pub fn install(&self, app_id: &str, yes: bool) -> Result<()> {
        self.ensure_flatpak()?;
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        args.push("flathub");
        args.push(app_id);

        println!("Installing Flatpak: {}", app_id);
        let mut cmd = command("flatpak", &args, false);
        run_inherit(&mut cmd, "flatpak install")?;
        self.history.log("flatpak-install", &[app_id.to_string()])?;
        Ok(())
    }

    pub fn remove(&self, app_id: &str, yes: bool) -> Result<()> {
        self.ensure_flatpak()?;
        let mut args = vec!["uninstall"];
        if yes {
            args.push("-y");
        }
        args.push(app_id);

        println!("Removing Flatpak: {}", app_id);
        let mut cmd = command("flatpak", &args, false);
        run_inherit(&mut cmd, "flatpak uninstall")?;
        self.history.log("flatpak-remove", &[app_id.to_string()])?;
        Ok(())
    }

    pub fn update(&self, yes: bool) -> Result<()> {
        self.ensure_flatpak()?;
        let mut args = vec!["update"];
        if yes {
            args.push("-y");
        }

        println!("Updating Flatpaks...");
        let mut cmd = command("flatpak", &args, false);
        run_inherit(&mut cmd, "flatpak update")?;
        self.history.log("flatpak-update", &["all".to_string()])?;
        Ok(())
    }

    pub fn list(&self) -> Result<()> {
        self.ensure_flatpak()?;
        println!("Installed Flatpaks:");
        let mut cmd = command("flatpak", &["list"], false);
        let output = run_capture(&mut cmd, "flatpak list")?;
        println!("{}", output);
        Ok(())
    }

    pub fn info(&self, app_id: &str) -> Result<()> {
        self.ensure_flatpak()?;
        println!("Flatpak information for: {}", app_id);
        let mut cmd = command("flatpak", &["info", app_id], false);
        let output = run_capture(&mut cmd, "flatpak info")?;
        println!("{}", output);
        Ok(())
    }

    pub fn setup_flathub(&self) -> Result<()> {
        self.ensure_flatpak()?;
        println!("Setting up Flathub repository...");
        let mut cmd = command(
            "flatpak",
            &["remote-add", "--if-not-exists", "flathub", "https://flathub.org/repo/flathub.flatpakrepo"],
            false,
        );
        run_inherit(&mut cmd, "flatpak remote-add")?;
        println!("Flathub repository added successfully!");
        Ok(())
    }
}
