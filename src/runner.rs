use std::process::{Command, Stdio};

use anyhow::{Context, Result};

pub fn command(base: &str, args: &[&str], use_sudo: bool) -> Command {
    let mut cmd = if use_sudo {
        let mut c = Command::new("sudo");
        c.arg(base);
        c
    } else {
        Command::new(base)
    };
    cmd.args(args);
    cmd
}

pub fn run_inherit(cmd: &mut Command, label: &str) -> Result<()> {
    let status = cmd
        .stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit())
        .status()
        .with_context(|| format!("failed to run {label}"))?;

    if !status.success() {
        anyhow::bail!("{label} exited with status {status}");
    }
    Ok(())
}

pub fn run_capture(cmd: &mut Command, label: &str) -> Result<String> {
    let output = cmd
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .output()
        .with_context(|| format!("failed to run {label}"))?;

    if !output.status.success() {
        let stderr = String::from_utf8_lossy(&output.stderr);
        anyhow::bail!("{label} failed: {stderr}");
    }

    Ok(String::from_utf8_lossy(&output.stdout).to_string())
}

pub fn run_capture_allow_fail(cmd: &mut Command, label: &str) -> Result<Option<String>> {
    let output = cmd
        .stdout(Stdio::piped())
        .stderr(Stdio::piped())
        .output()
        .with_context(|| format!("failed to run {label}"))?;
    if !output.status.success() {
        return Ok(None);
    }
    Ok(Some(String::from_utf8_lossy(&output.stdout).to_string()))
}

