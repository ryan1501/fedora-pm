use std::fs;
use std::path::PathBuf;

use anyhow::Context;
use chrono::{DateTime, Local};
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HistoryEntry {
    pub action: String,
    pub items: Vec<String>,
    pub timestamp: DateTime<Local>,
}

#[derive(Debug, Clone)]
pub struct History {
    path: PathBuf,
}

impl History {
    pub fn new(path: PathBuf) -> Self {
        Self { path }
    }

    pub fn log(&self, action: &str, items: &[String]) -> anyhow::Result<()> {
        let mut history = self.read_all()?;
        history.push(HistoryEntry {
            action: action.to_string(),
            items: items.to_vec(),
            timestamp: Local::now(),
        });
        let json = serde_json::to_string_pretty(&history).context("failed to encode history")?;
        if let Some(parent) = self.path.parent() {
            fs::create_dir_all(parent)
                .with_context(|| format!("failed to create history dir {}", parent.display()))?;
        }
        fs::write(&self.path, json)
            .with_context(|| format!("failed to write history file {}", self.path.display()))?;
        Ok(())
    }

    fn read_all(&self) -> anyhow::Result<Vec<HistoryEntry>> {
        if !self.path.exists() {
            return Ok(Vec::new());
        }
        let data = fs::read_to_string(&self.path)
            .with_context(|| format!("failed to read history file {}", self.path.display()))?;
        let entries: Vec<HistoryEntry> =
            serde_json::from_str(&data).context("failed to parse history file")?;
        Ok(entries)
    }

    pub fn print(&self, limit: usize) -> anyhow::Result<()> {
        let entries = self.read_all()?;
        if entries.is_empty() {
            println!("No history found");
            return Ok(());
        }

        let take = entries.len().saturating_sub(limit);
        let recent = &entries[take..];
        println!("\nRecent package management history (last {} entries):", recent.len());
        for entry in recent.iter().rev() {
            println!(
                "  [{}] {}: {}",
                entry.timestamp.format("%Y-%m-%d %H:%M:%S"),
                entry.action,
                entry.items.join(", ")
            );
        }
        Ok(())
    }
}

