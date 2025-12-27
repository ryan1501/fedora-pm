use crate::runner::{command, run_capture};
use anyhow::Result;
use std::collections::{HashMap, HashSet};

pub struct DependencyManager {
    use_sudo: bool,
}

impl DependencyManager {
    pub fn new(use_sudo: bool) -> Self {
        Self { use_sudo }
    }

    pub fn show_tree(&self, package: &str) -> Result<()> {
        println!("Dependency tree for: {}", package);
        let deps = self.get_dependencies(package)?;
        self.print_tree(package, &deps, 0, &mut HashSet::new());
        Ok(())
    }

    pub fn show_reverse(&self, package: &str) -> Result<()> {
        println!("Reverse dependencies (what depends on {})", package);
        let mut cmd = command("dnf", &["repoquery", "--installed", "--whatrequires", package], false);
        let output = run_capture(&mut cmd, "dnf repoquery")?;

        let packages: Vec<&str> = output.lines().filter(|l| !l.is_empty()).collect();
        if packages.is_empty() {
            println!("  No packages depend on {}", package);
        } else {
            for pkg in packages {
                println!("  {}", pkg);
            }
        }
        Ok(())
    }

    fn get_dependencies(&self, package: &str) -> Result<HashMap<String, Vec<String>>> {
        let mut cmd = command("dnf", &["repoquery", "--requires", package], false);
        let output = run_capture(&mut cmd, "dnf repoquery")?;

        let mut deps = HashMap::new();
        let mut current_deps = Vec::new();

        for line in output.lines() {
            if !line.is_empty() {
                current_deps.push(line.to_string());
            }
        }

        deps.insert(package.to_string(), current_deps);
        Ok(deps)
    }

    fn print_tree(&self, package: &str, deps: &HashMap<String, Vec<String>>, level: usize, visited: &mut HashSet<String>) {
        let indent = "  ".repeat(level);

        if visited.contains(package) {
            println!("{}├─ {} (already shown)", indent, package);
            return;
        }

        visited.insert(package.to_string());

        if level > 0 {
            println!("{}├─ {}", indent, package);
        }

        if let Some(dependencies) = deps.get(package) {
            for dep in dependencies {
                self.print_tree(dep, deps, level + 1, visited);
            }
        }
    }
}
