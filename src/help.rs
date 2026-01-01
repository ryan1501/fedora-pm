/// Help module for fedora-pm
/// This module contains help text and help-related functions

pub const HELP_GENERAL_TEXT: &str = "
Fedora Package Manager - A comprehensive package management tool for Fedora

Usage:
  fedora-pm [OPTIONS] <COMMAND>

Common Commands:
  install     Install packages
  remove      Remove packages  
  update      Update packages
  search      Search for packages
  info        Get package information

Use 'fedora-pm help <command>' for detailed help on specific commands.
";

pub const HELP_INSTALL_TEXT: &str = "Install packages from repositories";
pub const HELP_REMOVE_TEXT: &str = "Remove packages from system";
pub const HELP_UPDATE_TEXT: &str = "Update installed packages";
pub const HELP_SEARCH_TEXT: &str = "Search for packages in repositories";
pub const HELP_KERNEL_TEXT: &str = "Manage kernel installations";
pub const HELP_DRIVER_TEXT: &str = "Manage device drivers";
pub const HELP_GAMING_TEXT: &str = "Install gaming-related packages";
pub const HELP_SECURITY_TEXT: &str = "Manage security updates";
pub const HELP_REPO_TEXT: &str = "Manage software repositories";
pub const HELP_EXPORT_TEXT: &str = "Export package list to file";
pub const HELP_IMPORT_TEXT: &str = "Import package list from file";
pub const HELP_DOWNLOAD_TEXT: &str = "Download packages without installing";
pub const HELP_INSTALL_OFFLINE_TEXT: &str = "Install packages from local RPM files";
pub const HELP_CHANGELOG_TEXT: &str = "View package changelog";
pub const HELP_WHATSNEW_TEXT: &str = "Show what's new in pending updates";
pub const HELP_SIZE_TEXT: &str = "Analyze disk space usage";
pub const HELP_SELF_UPDATE_TEXT: &str = "Manage self-updates";
pub const CLEAN_ORPHANS_TEXT: &str = "Remove orphaned packages";

pub const HELP_TEMPLATE: &str = "
Command: {command} {subcommand}

Usage: fedora-pm {command} {options}

Description:
{help_text}
";

/// Show help for a specific command
pub fn show_command_help(command: &str) {
    let help_text = match command {
        "install" => HELP_INSTALL_TEXT,
        "remove" => HELP_REMOVE_TEXT,
        "update" => HELP_UPDATE_TEXT,
        "search" => HELP_SEARCH_TEXT,
        "kernel" => HELP_KERNEL_TEXT,
        "driver" => HELP_DRIVER_TEXT,
        "gaming" => HELP_GAMING_TEXT,
        "security" => HELP_SECURITY_TEXT,
        "repo" => HELP_REPO_TEXT,
        "export" => HELP_EXPORT_TEXT,
        "import" => HELP_IMPORT_TEXT,
        "download" => HELP_DOWNLOAD_TEXT,
        "install-offline" => HELP_INSTALL_OFFLINE_TEXT,
        "changelog" => HELP_CHANGELOG_TEXT,
        "whats-new" => HELP_WHATSNEW_TEXT,
        "size" => HELP_SIZE_TEXT,
        "self-update" => HELP_SELF_UPDATE_TEXT,
        "clean-orphans" => CLEAN_ORPHANS_TEXT,
        _ => HELP_GENERAL_TEXT,
    };

    println!("{}", HELP_TEMPLATE
        .replace("{command}", command)
        .replace("{subcommand}", "")
        .replace("{options}", "")
        .replace("{help_text}", help_text)
    );
}