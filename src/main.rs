use clap::{Parser, Subcommand};
use std::process;

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
    },
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

fn main() {
    let cli = Cli::parse();
    
    match cli.command {
        Commands::Install { packages, yes } => {
            println!("Installing packages: {:?}", packages);
            if !yes {
                println!("Would install with confirmation");
            }
        },
        Commands::Remove { packages, yes } => {
            println!("Removing packages: {:?}", packages);
        },
        Commands::Update { packages, yes } => {
            if packages.is_empty() {
                println!("Updating all packages");
            } else {
                println!("Updating packages: {:?}", packages);
            }
        },
        Commands::Search { query } => {
            println!("Searching for: {}", query);
        },
        Commands::Info { package } => {
            println!("Getting info for: {}", package);
        },
        Commands::List { available, installed, all, pattern } => {
            println!("Listing packages");
        },
        Commands::Clean => {
            println!("Cleaning package cache");
        },
        Commands::History => {
            println!("Showing package history");
        },
        Commands::Kernel { action } => {
            match action {
                KernelAction::List => println!("Listing kernels"),
                KernelAction::Install { version, yes } => println!("Installing kernel"),
                KernelAction::Remove { versions, yes, keep_current } => println!("Removing kernels"),
                KernelAction::RemoveOld { keep_last, yes } => println!("Removing old kernels"),
                KernelAction::Info { package } => println!("Kernel info"),
            }
        },
        Commands::Driver { action } => {
            match action {
                DriverAction::Status => println!("Driver status"),
                DriverAction::Detect => println!("Detecting drivers"),
                DriverAction::InstallNvidia { version, cuda, yes } => println!("Installing NVIDIA driver"),
                DriverAction::RemoveNvidia { yes } => println!("Removing NVIDIA driver"),
                DriverAction::ListNvidia => println!("Listing NVIDIA packages"),
                DriverAction::CheckNvidia => println!("Checking NVIDIA"),
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
                SecurityAction::Check => println!("Checking security updates"),
                SecurityAction::List { severity } => println!("Listing security updates"),
                SecurityAction::Update { yes } => println!("Installing security updates"),
                SecurityAction::Info { advisory_id } => println!("Security info: {}", advisory_id),
                SecurityAction::Cve { cve_id } => println!("CVE info: {}", cve_id),
                SecurityAction::Audit => println!("Running security audit"),
            }
        },
        Commands::Download { packages, dest, with_deps } => {
            println!("Downloading packages: {:?}", packages);
        },
        Commands::InstallOffline { rpm_files, yes } => {
            println!("Installing offline packages: {:?}", rpm_files);
        },
        Commands::Changelog { package, limit } => {
            println!("Showing changelog for: {}", package);
        },
        Commands::WhatsNew => {
            println!("Showing what's new in updates");
        },
        Commands::Size { top, total, analyze } => {
            println!("Analyzing package sizes");
        },
        Commands::CleanOrphans { yes } => {
            println!("Cleaning orphan packages");
        },
        Commands::SelfUpdate { action } => {
            match action {
                SelfUpdateAction::Status => {
                    println!("Checking self-update status");
                    println!("Current version: 1.1.0");
                    println!("Update source: GitHub (not configured)");
                },
                SelfUpdateAction::Update { force, quiet } => {
                    if !quiet {
                        println!("Checking for updates...");
                    }
                    println!("Self-update not configured - GitHub repository needed");
                },
                SelfUpdateAction::Enable { frequency } => {
                    println!("Enabling automatic updates with frequency: {}", frequency);
                },
                SelfUpdateAction::Disable => {
                    println!("Disabling automatic updates");
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