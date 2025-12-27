use crate::history::{History, HistoryEntry};
use crate::package::PackageManager;
use anyhow::Result;
use std::fs;

pub struct RollbackManager {
    history: History,
    package_manager: PackageManager,
}

impl RollbackManager {
    pub fn new(history: History, package_manager: PackageManager) -> Self {
        Self {
            history,
            package_manager,
        }
    }

    pub fn rollback_last(&self, yes: bool) -> Result<()> {
        let entries = self.read_history()?;
        if entries.is_empty() {
            println!("No history to rollback");
            return Ok(());
        }

        let last = &entries[entries.len() - 1];
        self.rollback_entry(last, yes)
    }

    pub fn rollback_by_id(&self, id: usize, yes: bool) -> Result<()> {
        let entries = self.read_history()?;
        if id == 0 || id > entries.len() {
            anyhow::bail!("Invalid history ID: {}", id);
        }

        let entry = &entries[id - 1];
        self.rollback_entry(entry, yes)
    }

    fn rollback_entry(&self, entry: &HistoryEntry, yes: bool) -> Result<()> {
        println!(
            "Rolling back: {} {} ({})",
            entry.action,
            entry.items.join(", "),
            entry.timestamp.format("%Y-%m-%d %H:%M:%S")
        );

        match entry.action.as_str() {
            "install" => {
                println!("Removing packages that were installed...");
                self.package_manager.remove(&entry.items, yes)?;
            }
            "remove" => {
                println!("Reinstalling packages that were removed...");
                self.package_manager.install(&entry.items, yes)?;
            }
            "update" => {
                println!("Warning: Cannot automatically rollback updates.");
                println!("You may need to manually downgrade packages or use dnf history.");
            }
            _ => {
                println!("Cannot rollback action: {}", entry.action);
            }
        }

        Ok(())
    }

    fn read_history(&self) -> Result<Vec<HistoryEntry>> {
        let path = &self.history.path;
        if !path.exists() {
            return Ok(Vec::new());
        }
        let data = fs::read_to_string(path)?;
        let entries: Vec<HistoryEntry> = serde_json::from_str(&data)?;
        Ok(entries)
    }

    pub fn get_history(&self) -> &History {
        &self.history
    }

    pub fn list_history(&self) -> Result<()> {
        let entries = self.read_history()?;
        if entries.is_empty() {
            println!("No history found");
            return Ok(());
        }

        println!("\nHistory (most recent last):");
        for (i, entry) in entries.iter().enumerate() {
            println!(
                "  [{}] {} - {} {}",
                i + 1,
                entry.timestamp.format("%Y-%m-%d %H:%M:%S"),
                entry.action,
                entry.items.join(", ")
            );
        }
        Ok(())
    }
}
