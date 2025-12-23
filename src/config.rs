use std::fs;
use std::path::{Path, PathBuf};

use anyhow::Context;
use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    #[serde(default = "default_auto_clean")]
    pub auto_clean: bool,
    #[serde(default = "default_true")]
    pub parallel_downloads: bool,
    #[serde(default = "default_true")]
    pub fastest_mirror: bool,
    #[serde(default = "default_true")]
    pub color_output: bool,
    #[serde(default = "default_history_file")]
    pub history_file: PathBuf,
    #[serde(skip)]
    pub config_dir: PathBuf,
}

fn default_auto_clean() -> bool {
    false
}

fn default_true() -> bool {
    true
}

fn default_history_file() -> PathBuf {
    let dir = default_config_dir();
    dir.join("history.json")
}

fn default_config_dir() -> PathBuf {
    dirs::home_dir()
        .map(|home| home.join(".fedora-pm"))
        .expect("home directory not found")
}

impl Config {
    pub fn load(custom_dir: Option<PathBuf>) -> anyhow::Result<Self> {
        let config_dir = custom_dir.unwrap_or_else(default_config_dir);
        fs::create_dir_all(&config_dir)
            .with_context(|| format!("failed to create config directory at {}", config_dir.display()))?;

        let config_path = config_dir.join("config.json");

        if !config_path.exists() {
            let mut cfg = Self::default();
            cfg.config_dir = config_dir.clone();
            cfg.persist(&config_path)?;
            return Ok(cfg);
        }

        let data = fs::read_to_string(&config_path)
            .with_context(|| format!("failed to read config file {}", config_path.display()))?;

        let mut cfg: Config = serde_json::from_str(&data)
            .with_context(|| format!("invalid config JSON in {}", config_path.display()))?;
        cfg.config_dir = config_dir.clone();
        if cfg.history_file.as_os_str().is_empty() {
            cfg.history_file = default_history_file();
        }
        Ok(cfg)
    }

    fn persist(&self, path: &Path) -> anyhow::Result<()> {
        let json = serde_json::to_string_pretty(self).context("failed to serialize config")?;
        fs::write(path, json)
            .with_context(|| format!("failed to write config file {}", path.display()))?;
        Ok(())
    }
}

impl Default for Config {
    fn default() -> Self {
        Self {
            auto_clean: default_auto_clean(),
            parallel_downloads: default_true(),
            fastest_mirror: default_true(),
            color_output: default_true(),
            history_file: default_history_file(),
            config_dir: default_config_dir(),
        }
    }
}

