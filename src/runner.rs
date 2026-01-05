use std::process::{Command, Stdio};

use anyhow::{Context, Result};

pub fn command(base: &str, args: &[&str], use_sudo: bool) -> Command {
    let mut cmd = if use_sudo {
        let mut c = Command::new("sudo");
        c.arg("--non-interactive"); // Prevent password prompts in scripts
        c.arg(base);
        c
    } else {
        Command::new(base)
    };
    cmd.args(args);
    cmd
}

pub fn command_with_env(base: &str, args: &[&str], use_sudo: bool, env_vars: &[(&str, &str)]) -> Command {
    let mut cmd = command(base, args, use_sudo);
    for (key, value) in env_vars {
        cmd.env(key, value);
    }
    cmd
}

pub fn run_inherit(cmd: &mut Command, label: &str) -> Result<()> {
    // Set timeout to prevent hanging
    cmd.stdin(Stdio::inherit())
        .stdout(Stdio::inherit())
        .stderr(Stdio::inherit());

    let status = cmd
        .status()
        .with_context(|| format!("failed to execute {label}"))?;

    if !status.success() {
        match status.code() {
            Some(code) => anyhow::bail!("{label} failed with exit code {}", code),
            None => anyhow::bail!("{label} terminated by signal"),
        }
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

