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
mod cachyos;
mod nvidia;


#[derive(Parser, Debug)]
#[command(name = "fedorapm", about = "Fedora Package Manager (Rust)", version = "2.0.0")]
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
    CachyOS {
        #[command(subcommand)]
        action: CachyOSAction,
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
    InstallNvidia { 
        version: Option<String>, 
        #[arg(long)] cuda: bool, 
        #[arg(long)] toolkit: bool,
        #[arg(short, long)] yes: bool 
    },
    #[command(name = "remove-nvidia")]
    RemoveNvidia { #[arg(short, long)] yes: bool },
    #[command(name = "list-nvidia")]
    ListNvidia,
    #[command(name = "check-nvidia")]
    CheckNvidia,
    #[command(name = "cuda-status")]
    CudaStatus,
    #[command(name = "setup-dev")]
    SetupDev { #[arg(short, long)] yes: bool },
}

#[derive(Subcommand, Debug)]
pub enum GamingAction {
    Install { #[arg(short, long)] yes: bool },
    #[command(name = "install-cachyos")]
    InstallCachyOS { kernel: Option<String>, variant: Option<String>, #[arg(short, long)] yes: bool },
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

#[derive(Subcommand, Debug)]
pub enum CachyOSAction {
    Status,
    #[command(name = "list-kernels")]
    ListKernels,
    #[command(name = "install-kernel")]
    InstallKernel { 
        kernel: String, 
        variant: Option<String>,
        #[arg(short, long)] yes: bool 
    },
    #[command(name = "enable-repo")]
    EnableRepo { 
        repo_type: String, 
        #[arg(short, long)] yes: bool 
    },
    #[command(name = "disable-repo")]
    DisableRepo { 
        repo_type: String, 
        #[arg(short, long)] yes: bool 
    },
    #[command(name = "check-cpu")]
    CheckCpu,
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
    
    let result: Result<(), Box<dyn std::error::Error>> = match cli.command {
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
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let kernel_manager = kernel::KernelManager::new(cli.sudo, history);
            
            match action {
                KernelAction::List => {
                    kernel_manager.current()?;
                    kernel_manager.list_installed()?;
                    kernel_manager.list_available()?;
                    Ok(())
                },
                KernelAction::Install { version, yes } => {
                    kernel_manager.install(version.as_deref(), yes)?;
                    Ok(())
                },
                KernelAction::Remove { versions, yes, keep_current } => {
                    kernel_manager.remove(&versions, yes, keep_current)?;
                    Ok(())
                },
                KernelAction::RemoveOld { keep_last, yes } => {
                    kernel_manager.remove_old(keep_last, yes)?;
                    Ok(())
                },
                KernelAction::Info { package } => {
                    kernel_manager.info(Some(&package))?;
                    Ok(())
                },
            }
        },
        Commands::Driver { action } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let nvidia_manager = nvidia::NvidiaManager::new(cli.sudo, history.clone());
            let driver_manager = driver::DriverManager::new(cli.sudo, history);
            
            match action {
                DriverAction::Status => {
                    driver_manager.status()?;
                    Ok(())
                },
                DriverAction::Detect => {
                    nvidia_manager.detect_hardware()?;
                    Ok(())
                },
                DriverAction::InstallNvidia { version, cuda, toolkit, yes } => {
                    nvidia_manager.install_driver(version.as_deref(), cuda, toolkit, yes)?;
                    Ok(())
                },
                DriverAction::RemoveNvidia { yes } => {
                    nvidia_manager.remove_driver(yes)?;
                    Ok(())
                },
                DriverAction::ListNvidia => {
                    nvidia_manager.list_available_drivers()?;
                    Ok(())
                },
                DriverAction::CheckNvidia => {
                    nvidia_manager.check_driver_status()?;
                    Ok(())
                },
                DriverAction::CudaStatus => {
                    nvidia_manager.check_cuda_support()?;
                    Ok(())
                },
                DriverAction::SetupDev { yes } => {
                    nvidia_manager.setup_development(yes)?;
                    Ok(())
                },
            }
        },
        Commands::Gaming { action } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let cachyos_manager = cachyos::CachyOSManager::new(cli.sudo, history.clone());
            
            match action {
                GamingAction::Install { yes } => {
                    let gaming = gaming::GamingManager::new(cli.sudo, history);
                    gaming.install_meta(yes)?;
                    Ok(())
                },
                GamingAction::InstallCachyOS { kernel, variant, yes } => {
                    let kernel_type = kernel.as_deref().unwrap_or("default");
                    cachyos_manager.install_kernel(kernel_type, variant.as_deref(), yes)?;
                    Ok(())
                },
            }
        },
        Commands::Deps { package, tree, reverse } => {
            let dep_manager = deps::DependencyManager::new(cli.sudo);
            
            if tree {
                dep_manager.show_tree(&package)?;
            } else if reverse {
                dep_manager.show_reverse(&package)?;
            } else {
                // Show both by default
                dep_manager.show_tree(&package)?;
                println!();
                dep_manager.show_reverse(&package)?;
            }
            Ok(())
        },
        Commands::Rollback { id, yes: _ } => {
            if let Some(id) = id {
                println!("Rolling back to transaction {}", id);
            } else {
                println!("Listing rollback options");
            }
            Ok(())
        },
        Commands::Group { action } => {
            match action {
                GroupAction::List => {
                    println!("Listing groups");
                    Ok(())
                },
                GroupAction::Info { group } => {
                    println!("Group info for: {}", group);
                    Ok(())
                },
                GroupAction::Install { group, yes: _ } => {
                    println!("Installing group: {}", group);
                    Ok(())
                },
                GroupAction::Remove { group, yes: _ } => {
                    println!("Removing group: {}", group);
                    Ok(())
                },
            }
        },
        Commands::Doctor => {
            println!("Running system health check");
            Ok(())
        },
        Commands::Flatpak { action } => {
            match action {
                FlatpakAction::Search { query } => {
                    println!("Searching Flatpaks: {}", query);
                    Ok(())
                },
                FlatpakAction::Install { app_id, yes: _ } => {
                    println!("Installing Flatpak: {}", app_id);
                    Ok(())
                },
                FlatpakAction::Remove { app_id, yes: _ } => {
                    println!("Removing Flatpak: {}", app_id);
                    Ok(())
                },
                FlatpakAction::Update { yes: _ } => {
                    println!("Updating Flatpaks");
                    Ok(())
                },
                FlatpakAction::List => {
                    println!("Listing Flatpaks");
                    Ok(())
                },
                FlatpakAction::Info { app_id } => {
                    println!("Flatpak info: {}", app_id);
                    Ok(())
                },
                FlatpakAction::SetupFlathub => {
                    println!("Setting up Flathub");
                    Ok(())
                },
            }
        },
        Commands::Export { file, with_flatpak: _ } => {
            println!("Exporting packages to: {}", file);
            Ok(())
        },
        Commands::Import { file, with_flatpak: _ } => {
            println!("Importing packages from: {}", file);
            Ok(())
        },
        Commands::Repo { action } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let repo_manager = repo::RepoManager::new(cli.sudo, history);
            
            match action {
                RepoAction::List => {
                    repo_manager.list(false)?;
                    Ok(())
                },
                RepoAction::Info { repo_id } => {
                    repo_manager.info(&repo_id)?;
                    Ok(())
                },
                RepoAction::Enable { repo_id } => {
                    repo_manager.enable(&repo_id)?;
                    Ok(())
                },
                RepoAction::Disable { repo_id } => {
                    repo_manager.disable(&repo_id)?;
                    Ok(())
                },
                RepoAction::Refresh => {
                    repo_manager.refresh()?;
                    Ok(())
                },
                RepoAction::Add { name, url } => {
                    repo_manager.add(&name, &url)?;
                    Ok(())
                },
                RepoAction::Remove { repo_id } => {
                    repo_manager.remove(&repo_id)?;
                    Ok(())
                },
            }
        },
        Commands::Security { action } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let security = security::SecurityManager::new(cli.sudo, history);
            
            match action {
                SecurityAction::Check => {
                    security.check()?;
                    Ok(())
                },
                SecurityAction::List { severity } => {
                    security.list(severity.as_deref())?;
                    Ok(())
                },
                SecurityAction::Update { yes } => {
                    security.update(yes)?;
                    Ok(())
                },
                SecurityAction::Audit => {
                    security.audit()?;
                    Ok(())
                },
                SecurityAction::Cve { cve_id } => {
                    security.cve_check(&cve_id)?;
                    Ok(())
                },
                SecurityAction::Info { advisory_id } => {
                    security.info(&advisory_id)?;
                    Ok(())
                },
            }
        },
        Commands::Download { packages, dest, with_deps } => {
            validation::validate_package_list(&packages)?;
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let pkg_manager = package::PackageManager::new(cli.sudo, history);
            pkg_manager.download(&packages, Some(dest.as_deref().unwrap_or(".")), with_deps)?;
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
            pkg_manager.changelog(&package, limit.map(|l| l as i32))?;
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
                SelfUpdateAction::Update { force: _, quiet } => {
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
        Commands::CachyOS { action } => {
            let config = config::Config::load(cli.config_dir.as_deref().map(|s| s.into()))?;
            let history = history::History::new(config.history_file.clone());
            let cachyos_manager = cachyos::CachyOSManager::new(cli.sudo, history);
            
            match action {
                CachyOSAction::Status => {
                    cachyos_manager.get_status()?;
                    Ok(())
                },
                CachyOSAction::ListKernels => {
                    cachyos_manager.list_kernels()?;
                    Ok(())
                },
                CachyOSAction::InstallKernel { kernel, variant, yes } => {
                    cachyos_manager.install_kernel(&kernel, variant.as_deref(), yes)?;
                    Ok(())
                },
                CachyOSAction::EnableRepo { repo_type, yes } => {
                    cachyos_manager.enable_repository(&repo_type, yes)?;
                    Ok(())
                },
                CachyOSAction::DisableRepo { repo_type, yes } => {
                    cachyos_manager.disable_repository(&repo_type, yes)?;
                    Ok(())
                },
                CachyOSAction::CheckCpu => {
                    cachyos_manager.check_cpu_features()?;
                    Ok(())
                },
            }
        },
        Commands::Help { command } => {
            if let Some(cmd) = command {
                println!("Help for command: {}", cmd);
            } else {
                // Print general help
                println!("The Modern Fedora Package Manager");
                println!("");
                println!("Usage:");
                println!("  fedorapm [OPTIONS] <COMMAND>");
                println!("");
                println!("Get help for a specific command:");
                println!("  fedorapm <COMMAND> --help");
                println!("");
                println!("Examples:");
                println!("  fedorapm install vim");
                println!("  fedorapm gaming install");
                println!("  fedorapm kernel list");
                println!("");
                process::exit(0);
            }
        },
    };
    
    result
}