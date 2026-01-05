use clap::{Parser, Subcommand};
use std::process;
use anyhow::Result;

mod help;
mod gaming;
mod flatpak;
mod export;
mod driver;
mod download;
mod security;
mod doctor;
mod runner;
mod diskspace;
mod deps;
mod rollback;
mod config;
mod changelog;
mod repo;
mod package;
mod history;
mod kernel;
mod groups;
mod validation;


#[derive(Parser, Debug)]
#[command(name = "fedora-pm", about = "Fedora Package Manager (Rust)", version = "1.1.0")]
pub struct Cli {
    #[command(subcommand)]
    pub command: Commands,
    
    #[arg(short = 'v', long = "verbose")]
    pub verbose: bool,
    
    #[arg(short, long)]
    pub quiet: bool,
    
    #[arg(long, default_value = "true")]
    pub sudo: bool,
    
    #[arg(long)]
    pub config_dir: Option<String>,
    
    #[arg(long, help = "Launch GUI instead of CLI")]
    pub gui: bool,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    Install {
        #[arg()]
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Remove {
        #[arg()]
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Update {
        #[arg()]
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Search { 
        query: String 
    },
    Info { 
        package: String 
    },
    List {
        #[arg(short, long)]
        available: bool,
        #[arg(short, long)]
        installed: bool,
        #[arg(short, long)]
        all: bool,
        #[arg()]
        pattern: Option<String>,
    },
    Clean,
    History,
    Kernel {
        #[command(subcommand)]
        action: KernelAction,
    },
    Driver {
        #[command(subcommand)]
        action: DriverAction,
    },
    Gaming {
        #[command(subcommand)]
        action: GamingAction,
    },
    Deps {
        package: String,
        #[arg(short, long)]
        tree: bool,
        #[arg(short, long)]
        reverse: bool,
    },
    Rollback {
        id: Option<usize>,
        #[arg(short, long)]
        yes: bool,
    },
    Group {
        #[command(subcommand)]
        action: GroupAction,
    },
    Doctor,
    Flatpak {
        #[command(subcommand)]
        action: FlatpakAction,
    },
    Export {
        file: String,
        #[arg(long)]
        with_flatpak: bool,
    },
    Import {
        file: String,
        #[arg(long)]
        with_flatpak: bool,
    },
    Repo {
        #[command(subcommand)]
        action: RepoAction,
    },
    Security {
        #[command(subcommand)]
        action: SecurityAction,
    },
    Download {
        #[arg()]
        packages: Vec<String>,
        #[arg(short, long)]
        dest: Option<String>,
        #[arg(short, long)]
        with_deps: bool,
    },
    #[command(name = "install-offline")]
    InstallOffline {
        #[arg()]
        rpm_files: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Changelog {
        package: String,
        #[arg(long)]
        limit: Option<usize>,
    },
    #[command(name = "whats-new")]
    WhatsNew,
    Size {
        #[arg(long)]
        top: Option<usize>,
        #[arg(long)]
        total: bool,
        #[arg(long)]
        analyze: bool,
    },
    #[command(name = "clean-orphans")]
    CleanOrphans {
        #[arg(short, long)]
        yes: bool,
    },
    #[command(name = "self-update")]
    SelfUpdate {
        #[command(subcommand)]
        action: SelfUpdateAction,
    },
    Help {
        command: Option<String>,
    }
}

#[derive(Subcommand, Debug)]
pub enum KernelAction {
    List,
    Install { version: Option<String>, yes: bool },
    Remove { versions: Vec<String>, yes: bool, #[arg(long)] keep_current: bool },
    #[command(name = "remove-old")]
    RemoveOld { #[arg(long, default_value = "2")] keep_last: usize, #[arg(short, long)] yes: bool },
    Info { package: String },
}

#[derive(Subcommand, Debug)]
pub enum DriverAction {
    Status,
    Detect,
    #[command(name = "install-nvidia")]
    InstallNvidia { version: Option<String>, #[arg(long)] cuda: bool, #[arg(short, long)] yes: bool },
    #[command(name = "remove-nvidia")]
    RemoveNvidia { #[arg(short, long)] yes: bool },
    #[command(name = "list-nvidia")]
    ListNvidia,
    #[command(name = "check-nvidia")]
    CheckNvidia,
}

#[derive(Subcommand, Debug)]
pub enum GamingAction {
    Install { #[arg(short, long)] yes: bool },
}

#[derive(Subcommand, Debug)]
pub enum GroupAction {
    List,
    Info { group: String },
    Install { group: String, #[arg(short, long)] yes: bool },
    Remove { group: String, #[arg(short, long)] yes: bool },
}

#[derive(Subcommand, Debug)]
pub enum FlatpakAction {
    Search { query: String },
    Install { app_id: String, #[arg(short, long)] yes: bool },
    Remove { app_id: String, #[arg(short, long)] yes: bool },
    Update { #[arg(short, long)] yes: bool },
    List,
    Info { app_id: String },
    #[command(name = "setup-flathub")]
    SetupFlathub,
}

#[derive(Subcommand, Debug)]
pub enum RepoAction {
    List,
    Info { repo_id: String },
    Enable { repo_id: String },
    Disable { repo_id: String },
    Refresh,
    Add { name: String, url: String },
    Remove { repo_id: String },
}

#[derive(Subcommand, Debug)]
pub enum SecurityAction {
    Check,
    List { severity: Option<String> },
    Update { #[arg(short, long)] yes: bool },
    Info { advisory_id: String },
    Cve { cve_id: String },
    Audit,
}

#[derive(Subcommand, Debug)]
pub enum SelfUpdateAction {
    Status,
    Update {
        #[arg(short, long)]
        force: bool,
        #[arg(short, long)]
        quiet: bool,
    },
    Enable { frequency: String },
    Disable,
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let cli = Cli::parse();
    
    // GUI not yet implemented
    if cli.gui {
        eprintln!("GUI is not yet implemented in the native Rust version.");
        eprintln!("Use the CLI interface instead:");
        eprintln!("  fedora-pm --help");
        std::process::exit(1);
    }
    
    let result =     match cli.command {
        Commands::Install { packages, yes } => {
            validation::validate_package_list(&packages)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.install(&packages, yes)?;
            Ok(())
        },
        Commands::Remove { packages, yes } => {
            validation::validate_package_list(&packages)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.remove(&packages, yes)?;
            Ok(())
        },
        Commands::Update { packages, yes } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.update(&packages, yes)?;
            Ok(())
        },
        Commands::Search { query } => {
            validation::validate_search_query(&query)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.search(&query)?;
            Ok(())
        },
        Commands::Info { package } => {
            validation::validate_package_name(&package)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.info(&package)?;
            Ok(())
        },
        Commands::List { available, installed, all, pattern } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            
            if all || installed {
                pkg_manager.list_installed(pattern.as_deref())?;
            }
            if all || available {
                pkg_manager.list_available(pattern.as_deref())?;
            }
            if !all && !installed && !available {
                pkg_manager.list_installed(pattern.as_deref())?;
            }
            Ok(())
        },
        Commands::Clean => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.clean(true, true)?; // Clean both cache and metadata
            Ok(())
        },
        Commands::History => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            history.print(10)?; // Show last 10 entries
            Ok(())
        },
        Commands::Kernel { action } => {
            match action {
                KernelAction::List => {
                    println!("Listing kernels");
                    Ok(())
                },
                KernelAction::Install { version, yes } => {
                    println!("Installing kernel: {}", version.as_ref().unwrap_or(&"latest".to_string()));
                    Ok(())
                },
                KernelAction::Remove { versions, yes, keep_current } => {
                    println!("Removing kernels: {:?}", versions);
                    Ok(())
                },
                KernelAction::RemoveOld { keep_last, yes } => {
                    println!("Removing old kernels, keeping last {}", keep_last);
                    Ok(())
                },
                KernelAction::Info { package } => {
                    println!("Kernel info: {}", package);
                    Ok(())
                },
            }
        },
        Commands::Driver { action } => {
            match action {
                DriverAction::Status => {
                    println!("Checking driver status...");
                    Ok(())
                },
                DriverAction::Detect => {
                    println!("Detecting GPU drivers...");
                    Ok(())
                },
                DriverAction::InstallNvidia { version, cuda, yes } => {
                    println!("Installing NVIDIA driver: version={:?}, cuda={}", version, cuda);
                    Ok(())
                },
                DriverAction::RemoveNvidia { yes } => {
                    println!("Removing NVIDIA driver...");
                    Ok(())
                },
            }
        },
        Commands::Gaming { action } => {
            match action {
                GamingAction::Install { yes } => println!("Installing gaming packages"),
            }
        },
        Commands::Deps { package, tree, reverse } => {
            println!("Showing dependencies for: {}", package);
        },
        Commands::Rollback { id, yes } => {
            if let Some(id) = id {
                println!("Rolling back to transaction {}", id);
            } else {
                println!("Listing rollback options");
            }
        },
        Commands::Group { action } => {
            match action {
                GroupAction::List => println!("Listing groups"),
                GroupAction::Info { group } => println!("Group info for: {}", group),
                GroupAction::Install { group, yes } => println!("Installing group: {}", group),
                GroupAction::Remove { group, yes } => println!("Removing group: {}", group),
            }
        },
        Commands::Doctor => {
            println!("Running system health check");
        },
        Commands::Flatpak { action } => {
            match action {
                FlatpakAction::Search { query } => println!("Searching Flatpaks: {}", query),
                FlatpakAction::Install { app_id, yes } => println!("Installing Flatpak: {}", app_id),
                FlatpakAction::Remove { app_id, yes } => println!("Removing Flatpak: {}", app_id),
                FlatpakAction::Update { yes } => println!("Updating Flatpaks"),
                FlatpakAction::List => println!("Listing Flatpaks"),
                FlatpakAction::Info { app_id } => println!("Flatpak info: {}", app_id),
                FlatpakAction::SetupFlathub => println!("Setting up Flathub"),
            }
        },
        Commands::Export { file, with_flatpak } => {
            println!("Exporting packages to: {}", file);
        },
        Commands::Import { file, with_flatpak } => {
            println!("Importing packages from: {}", file);
        },
        Commands::Repo { action } => {
            match action {
                RepoAction::List => println!("Listing repositories"),
                RepoAction::Info { repo_id } => println!("Repo info: {}", repo_id),
                RepoAction::Enable { repo_id } => println!("Enabling repo: {}", repo_id),
                RepoAction::Disable { repo_id } => println!("Disabling repo: {}", repo_id),
                RepoAction::Refresh => println!("Refreshing repositories"),
                RepoAction::Add { name, url } => println!("Adding repo: {} -> {}", name, url),
                RepoAction::Remove { repo_id } => println!("Removing repo: {}", repo_id),
            }
        },
        Commands::Security { action } => {
            match action {
                SecurityAction::Check => {
                    println!("Checking for security updates...");
                    Ok(())
                },
                SecurityAction::List { severity, advisory_id, cve_id } => {
                    println!("Listing security updates: severity={:?}, advisory_id={:?}, cve_id={:?}", severity, advisory_id, cve_id);
                    Ok(())
                },
                SecurityAction::Update => {
                    println!("Installing security updates...");
                    Ok(())
                },
                SecurityAction::Audit => {
                    println!("Running security audit...");
                    Ok(())
                },
                SecurityAction::Cve { cve_id } => {
                    println!("Checking CVE: {}", cve_id);
                    Ok(())
                },
                SecurityAction::Info { advisory_id } => {
                    println!("Advisory info: {}", advisory_id);
                    Ok(())
                },
            }
        },
        Commands::Download { packages, dest, with_deps } => {
            validation::validate_package_list(&packages)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.download(&packages, dest.as_deref().unwrap_or("."), with_deps)?;
            Ok(())
        },
        Commands::InstallOffline { rpm_files, yes } => {
            validation::validate_package_list(&rpm_files)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.install_offline(&rpm_files, yes)?;
            Ok(())
        },
        Commands::Changelog { package, limit } => {
            validation::validate_package_name(&package)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.changelog(&package, limit)?;
            Ok(())
        },
        Commands::WhatsNew => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.whats_new()?;
            Ok(())
        },
        Commands::Size { top, total, analyze } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.size(top, total, analyze)?;
            Ok(())
        },
        Commands::CleanOrphans { yes } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.clean_orphans(yes)?;
            Ok(())
        },
        Commands::SelfUpdate { action } => {
            match action {
                SelfUpdateAction::Status => {
                    println!("Checking self-update status");
                    println!("Current version: {}", env!("CARGO_PKG_VERSION"));
                    println!("Update source: GitHub (not configured)");
                    Ok(())
                },
                SelfUpdateAction::Update { force, quiet } => {
                    if !quiet {
                        println!("Checking for updates...");
                    }
                    println!("Self-update not configured - GitHub repository needed");
                    Ok(())
                },
                SelfUpdateAction::Enable { frequency } => {
                    println!("Enabling automatic updates with frequency: {}", frequency);
                    Ok(())
                },
                SelfUpdateAction::Disable => {
                    println!("Disabling automatic updates");
                    Ok(())
                },
            }
        },
        Commands::Help { command } => {
            if let Some(cmd) = command {
                println!("Help for command: {}", cmd);
            } else {
                // Print general help
                println!("{}", help::HELP_GENERAL_TEXT);
                process::exit(0);
            }
        },
    }
}