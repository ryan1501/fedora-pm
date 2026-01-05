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
        println!("=== Dependency Tree for: {} ===", package);
        
        // Check if package exists first
        let mut cmd = command("dnf", &["info", package], false);
        if run_capture(&mut cmd, "dnf info").is_err() {
            println!("Package '{}' not found", package);
            return Ok(());
        }
        
        let deps = self.get_dependencies(package)?;
        if deps.is_empty() {
            println!("No dependencies found");
        } else {
            println!("Direct dependencies:");
            self.print_tree(package, &deps, 0, &mut HashSet::new());
        }
        
        // Show reverse dependencies too
        println!("\n=== Reverse Dependencies (what depends on {}) ===", package);
        self.show_reverse(package)?;
        
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
            let trimmed = line.trim();
            if !trimmed.is_empty() && !trimmed.starts_with("warning:") {
                // Filter out system libraries and only show package dependencies
                if !trimmed.contains(".so") && !trimmed.contains("(") && trimmed.contains('-') {
                    current_deps.push(trimmed.to_string());
                }
            }
        }

        deps.insert(package.to_string(), current_deps);
        Ok(deps)
    }

    fn print_tree(&self, package: &str, deps: &HashMap<String, Vec<String>>, level: usize, visited: &mut HashSet<String>) {
        let indent = "  ".repeat(level);

        if visited.contains(package) {
            println!("{}└─ {} (circular/already shown)", indent, package);
            return;
        }

        visited.insert(package.to_string());

        if level > 0 {
            if level == 1 {
                println!("└─ {}", package);
            } else {
                println!("{}└─ {}", indent, package);
            }
        }

        if let Some(dependencies) = deps.get(package) {
            for (i, dep) in dependencies.iter().enumerate() {
                let is_last = i == dependencies.len() - 1;
                let new_indent = if level == 0 { "  ".to_string() } else { indent.clone() + "  " };
                
                if is_last {
                    println!("{}└─ {}", new_indent, dep);
                } else {
                    println!("{}├─ {}", new_indent, dep);
                }
                
                // Recursively show dependencies of dependencies (limited depth to avoid infinite loops)
                if level < 3 {
                    if let Ok(sub_deps) = self.get_single_dependencies(dep) {
                        self.print_dependency_level(&sub_deps, &new_indent, level + 1, visited);
                    }
                }
            }
        }
    }

    fn get_single_dependencies(&self, package: &str) -> Result<Vec<String>> {
        let mut cmd = command("dnf", &["repoquery", "--requires", package], false);
        let output = run_capture(&mut cmd, "dnf repoquery")?;
        
        let mut deps = Vec::new();
        for line in output.lines() {
            let trimmed = line.trim();
            if !trimmed.is_empty() && !trimmed.starts_with("warning:") {
                if !trimmed.contains(".so") && !trimmed.contains("(") && trimmed.contains('-') {
                    deps.push(trimmed.to_string());
                }
            }
        }
        Ok(deps)
    }

    fn print_dependency_level(&self, deps: &[String], indent: &str, _level: usize, visited: &mut HashSet<String>) {
        for (i, dep) in deps.iter().enumerate() {
            let is_last = i == deps.len() - 1;
            
            if visited.contains(dep) {
                if is_last {
                    println!("{}└─ {} (already shown)", indent, dep);
                } else {
                    println!("{}├─ {} (already shown)", indent, dep);
                }
                continue;
            }

            if is_last {
                println!("{}└─ {}", indent, dep);
            } else {
                println!("{}├─ {}", indent, dep);
            }
        }
    }
}
