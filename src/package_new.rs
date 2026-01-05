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

        anyhow::bail!("package {} not found", package);
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

    pub fn download(&self, packages: &[String], dest: Option<&str>, with_deps: bool) -> Result<()> {
        let mut args = vec!["download"];
        if with_deps {
            args.push("--resolve");
        }
        for pkg in packages {
            args.push(pkg);
        }
        
        let mut cmd = if let Some(dest_path) = dest {
            let mut dest_args = vec!["--destdir", dest_path];
            dest_args.extend(args);
            command("dnf", &dest_args, false)
        } else {
            command("dnf", &args, false)
        };
        
        println!("Downloading packages: {}", packages.join(", "));
        run_inherit(&mut cmd, "dnf download")?;
        self.history.log("download", packages)?;
        Ok(())
    }

    pub fn install_offline(&self, rpm_files: &[String], yes: bool) -> Result<()> {
        if rpm_files.is_empty() {
            anyhow::bail!("no RPM files specified");
        }
        
        let mut args = vec!["install"];
        if yes {
            args.push("-y");
        }
        for rpm in rpm_files {
            args.push(rpm);
        }
        
        println!("Installing RPM files: {}", rpm_files.join(", "));
        let mut cmd = command("dnf", &args, self.use_sudo);
        run_inherit(&mut cmd, "dnf install offline")?;
        self.history.log("install-offline", rpm_files)?;
        Ok(())
    }

    pub fn changelog(&self, package: &str, limit: Option<i32>) -> Result<()> {
        let mut args = vec!["changelog", package];
        if let Some(l) = limit {
            args.push(&format!("--count={}", l));
        }
        
        let mut cmd = command("dnf", &args, false);
        let output = run_capture(&mut cmd, "dnf changelog")?;
        println!("{output}");
        Ok(())
    }

    pub fn whats_new(&self) -> Result<()> {
        let mut cmd = command("dnf", &["updateinfo"], false);
        let output = run_capture(&mut cmd, "dnf updateinfo")?;
        println!("{output}");
        Ok(())
    }

    pub fn size(&self, top: Option<usize>, total: bool, analyze: bool) -> Result<()> {
        let mut args = vec!["list", "installed"];
        if analyze {
            args.push("--info");
        }
        
        let mut cmd = command("dnf", &args, false);
        let output = run_capture(&mut cmd, "dnf list installed")?;
        
        if total {
            let lines: Vec<&str> = output.lines().collect();
            let mut total_size: f64 = 0.0;
            for line in lines {
                if line.contains("Size") {
                    if let Some(size_part) = line.split_whitespace().find(|s| s.ends_with("M") || s.ends_with("K")) {
                        if let Some(num_str) = size_part.trim_end_matches(|c| c == 'M' || c == 'K').strip_suffix(|c| c == 'M' || c == 'K') {
                            if let Ok(num) = num_str.parse::<f64>() {
                                total_size += if size_part.ends_with("M") { num * 1024.0 * 1024.0 } else { num * 1024.0 };
                            }
                        }
                    }
                }
            }
            println!("Total installed package size: {:.2} MB", total_size / (1024.0 * 1024.0));
        }
        
        if let Some(top_count) = top {
            let lines: Vec<&str> = output.lines().collect();
            let mut packages = Vec::new();
            
            for line in lines.iter().skip(1) {
                if let Some(pkg_info) = line.split_whitespace().next() {
                    packages.push(pkg_info.to_string());
                }
            }
            
            packages.sort();
            packages.truncate(top_count);
            
            println!("Top {} largest installed packages:", top_count);
            for (i, pkg) in packages.iter().enumerate() {
                println!("  {}. {}", i + 1, pkg);
            }
        }
        
        Ok(())
    }

    pub fn clean_orphans(&self, yes: bool) -> Result<()> {
        let mut cmd = command("dnf", &["autoremove"], self.use_sudo);
        if yes {
            cmd.arg("-y");
        }
        
        println!("Removing orphan packages...");
        run_inherit(&mut cmd, "dnf autoremove")?;
        self.history.log("clean-orphans", &["autoremove".to_string()])?;
        Ok(())
    }

    pub fn show_history(&self) -> Result<()> {
        self.history.print(10)
    }
}