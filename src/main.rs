mod changelog;
mod config;
mod deps;
mod diskspace;
mod doctor;
mod download;
mod driver;
mod export;
mod flatpak;
mod gaming;
mod groups;
mod history;
mod kernel;
mod package;
mod repo;
mod rollback;
mod runner;
mod security;

use clap::{Parser, Subcommand};
use std::path::PathBuf;

use crate::{
    changelog::ChangelogManager,
    config::Config,
    deps::DependencyManager,
    diskspace::DiskSpaceManager,
    doctor::DoctorManager,
    download::DownloadManager,
    driver::DriverManager,
    export::ExportManager,
    flatpak::FlatpakManager,
    gaming::GamingManager,
    groups::GroupManager,
    history::History,
    kernel::KernelManager,
    package::PackageManager,
    repo::RepoManager,
    rollback::RollbackManager,
    security::SecurityManager,
};

#[derive(Parser, Debug)]
#[command(name = "fedora-pm", about = "Fedora Package Manager (Rust)", version)]
struct Cli {
    /// Path to configuration directory (defaults to ~/.fedora-pm)
    #[arg(long)]
    config_dir: Option<PathBuf>,

    /// Run package operations without sudo (for debugging)
    #[arg(long, default_value_t = true, action = clap::ArgAction::Set, value_name = "BOOL")]
    sudo: bool,

    /// Verbose output
    #[arg(short, long, action = clap::ArgAction::Count)]
    verbose: u8,

    /// Quiet mode (minimal output)
    #[arg(short, long)]
    quiet: bool,

    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    Install {
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Remove {
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Update {
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Search { query: String },
    Info { package: String },
    List {
        #[command(subcommand)]
        list: ListCommand,
    },
    Clean {
        #[arg(long, default_value_t = true)]
        cache: bool,
        #[arg(long, default_value_t = true)]
        metadata: bool,
    },
    History {
        #[arg(short = 'n', long, default_value_t = 10)]
        limit: usize,
    },
    Kernel {
        #[command(subcommand)]
        kernel: KernelCommand,
    },
    Driver {
        #[command(subcommand)]
        driver: DriverCommand,
    },
    Gaming {
        #[command(subcommand)]
        gaming: GamingCommand,
    },
    /// Show dependency tree for a package
    Deps {
        package: String,
        #[arg(long)]
        tree: bool,
        #[arg(long)]
        reverse: bool,
    },
    /// Rollback package operations
    Rollback {
        #[arg(long)]
        id: Option<usize>,
        #[arg(short, long)]
        yes: bool,
    },
    /// Manage package groups
    Group {
        #[command(subcommand)]
        group: GroupCommand,
    },
    /// Run system health check
    Doctor,
    /// Manage Flatpak applications
    Flatpak {
        #[command(subcommand)]
        flatpak: FlatpakCommand,
    },
    /// Export/import package lists
    Export {
        file: String,
        #[arg(long)]
        with_flatpak: bool,
    },
    Import {
        file: String,
        #[arg(short, long)]
        yes: bool,
    },
    /// Manage repositories
    Repo {
        #[command(subcommand)]
        repo: RepoCommand,
    },
    /// Security updates and audits
    Security {
        #[command(subcommand)]
        security: SecurityCommand,
    },
    /// Download packages
    Download {
        packages: Vec<String>,
        #[arg(long)]
        dest: Option<String>,
        #[arg(long)]
        with_deps: bool,
    },
    /// Install from downloaded RPMs
    InstallOffline {
        rpm_files: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    /// View package changelog
    Changelog {
        package: String,
        #[arg(short = 'n', long)]
        limit: Option<usize>,
    },
    /// Show what's new in pending updates
    Whatsnew,
    /// Disk space analysis
    Size {
        #[arg(long)]
        top: Option<usize>,
        #[arg(long)]
        total: bool,
        #[arg(long)]
        analyze: bool,
    },
    /// Find and remove orphaned packages
    CleanOrphans {
        #[arg(short, long)]
        yes: bool,
    },
}

#[derive(Subcommand, Debug)]
enum ListCommand {
    Installed { pattern: Option<String> },
    Available { pattern: Option<String> },
}

#[derive(Subcommand, Debug)]
enum KernelCommand {
    Current,
    List {
        #[arg(long)]
        available: bool,
    },
    Install {
        version: Option<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Remove {
        versions: Vec<String>,
        #[arg(short, long)]
        yes: bool,
        #[arg(long, default_value_t = true)]
        keep_current: bool,
    },
    RemoveOld {
        #[arg(long, default_value_t = 2)]
        keep: usize,
        #[arg(short, long)]
        yes: bool,
    },
    Info {
        version: Option<String>,
    },
    Cachyos {
        #[command(subcommand)]
        command: CachyosCommand,
    },
}

#[derive(Subcommand, Debug)]
enum CachyosCommand {
    List,
    Enable {
        #[arg(value_parser = ["gcc", "lto", "both"], default_value = "gcc")]
        r#type: String,
        #[arg(short, long)]
        yes: bool,
    },
    Check,
    Install {
        #[arg(value_parser = ["default", "lts", "rt", "server"], default_value = "default")]
        r#type: String,
        #[arg(value_parser = ["gcc", "lto"], default_value = "gcc")]
        build: String,
        #[arg(short, long)]
        yes: bool,
    },
    CheckCpu,
}

#[derive(Subcommand, Debug)]
enum DriverCommand {
    Status,
    Detect,
    InstallNvidia {
        #[arg(long)]
        version: Option<String>,
        #[arg(long)]
        cuda: bool,
        #[arg(short, long)]
        yes: bool,
    },
    RemoveNvidia {
        #[arg(short, long)]
        yes: bool,
    },
    ListNvidia,
    CheckNvidia,
    InstallCuda {
        #[arg(short, long)]
        yes: bool,
    },
}

#[derive(Subcommand, Debug)]
enum GamingCommand {
    Install {
        #[arg(short, long)]
        yes: bool,
    },
}

#[derive(Subcommand, Debug)]
enum GroupCommand {
    List,
    Info { group: String },
    Install {
        group: String,
        #[arg(short, long)]
        yes: bool,
    },
    Remove {
        group: String,
        #[arg(short, long)]
        yes: bool,
    },
}

#[derive(Subcommand, Debug)]
enum FlatpakCommand {
    Search { query: String },
    Install {
        app_id: String,
        #[arg(short, long)]
        yes: bool,
    },
    Remove {
        app_id: String,
        #[arg(short, long)]
        yes: bool,
    },
    Update {
        #[arg(short, long)]
        yes: bool,
    },
    List,
    Info { app_id: String },
    SetupFlathub,
}

#[derive(Subcommand, Debug)]
enum RepoCommand {
    List {
        #[arg(long)]
        all: bool,
    },
    Enable { repo_id: String },
    Disable { repo_id: String },
    Add { name: String, url: String },
    Remove { repo_id: String },
    Info { repo_id: String },
    Refresh,
}

#[derive(Subcommand, Debug)]
enum SecurityCommand {
    Check,
    List {
        #[arg(long)]
        severity: Option<String>,
    },
    Update {
        #[arg(short, long)]
        yes: bool,
    },
    Info { advisory_id: String },
    Cve { cve_id: String },
    Audit,
}

struct App {
    _config: Config,
    history: History,
    use_sudo: bool,
}

impl App {
    fn new(config_dir: Option<PathBuf>, use_sudo: bool) -> anyhow::Result<Self> {
        let config = Config::load(config_dir)?;
        let history = History::new(config.history_file.clone());
        Ok(Self {
            _config: config,
            history,
            use_sudo,
        })
    }
}

fn main() -> anyhow::Result<()> {
    let cli = Cli::parse();

    // Setup logging
    let log_level = match cli.verbose {
        0 => "error",
        1 => "warn",
        2 => "info",
        _ => "debug",
    };

    if !cli.quiet {
        env_logger::Builder::from_env(env_logger::Env::default().default_filter_or(log_level))
            .init();
    }

    let app = App::new(cli.config_dir, cli.sudo)?;
    let pkg = PackageManager::new(app.use_sudo, app.history.clone());
    let kernel = KernelManager::new(app.use_sudo, app.history.clone());
    let driver = DriverManager::new(app.use_sudo, app.history.clone());
    let gaming_mgr = GamingManager::new(app.use_sudo, app.history.clone());
    let deps = DependencyManager::new(app.use_sudo);
    let rollback = RollbackManager::new(app.history.clone(), pkg.clone());
    let groups = GroupManager::new(app.use_sudo, app.history.clone());
    let doctor = DoctorManager::new(app.use_sudo);
    let flatpak = FlatpakManager::new(app.use_sudo, app.history.clone());
    let export = ExportManager::new(app.use_sudo);
    let repo = RepoManager::new(app.use_sudo, app.history.clone());
    let security = SecurityManager::new(app.use_sudo, app.history.clone());
    let download = DownloadManager::new(app.use_sudo, app.history.clone());
    let changelog = ChangelogManager::new(app.use_sudo);
    let diskspace = DiskSpaceManager::new(app.use_sudo);

    match cli.command {
        Commands::Install { packages, yes } => pkg.install(&packages, yes)?,
        Commands::Remove { packages, yes } => pkg.remove(&packages, yes)?,
        Commands::Update { packages, yes } => pkg.update(&packages, yes)?,
        Commands::Search { query } => pkg.search(&query)?,
        Commands::Info { package } => pkg.info(&package)?,
        Commands::List { list } => match list {
            ListCommand::Installed { pattern } => pkg.list_installed(pattern.as_deref())?,
            ListCommand::Available { pattern } => pkg.list_available(pattern.as_deref())?,
        },
        Commands::Clean { cache, metadata } => pkg.clean(cache, metadata)?,
        Commands::History { limit } => app.history.print(limit)?,
        Commands::Kernel { kernel: command } => match command {
            KernelCommand::Current => kernel.current()?,
            KernelCommand::List { available } => {
                if available {
                    kernel.list_available()?;
                } else {
                    kernel.list_installed()?;
                }
            }
            KernelCommand::Install { version, yes } => kernel.install(version.as_deref(), yes)?,
            KernelCommand::Remove {
                versions,
                yes,
                keep_current,
            } => kernel.remove(&versions, yes, keep_current)?,
            KernelCommand::RemoveOld { keep, yes } => kernel.remove_old(keep, yes)?,
            KernelCommand::Info { version } => kernel.info(version.as_deref())?,
            KernelCommand::Cachyos { command } => match command {
                CachyosCommand::List => kernel.cachyos_list()?,
                CachyosCommand::Enable { r#type, yes } => kernel.cachyos_enable(&r#type, yes)?,
                CachyosCommand::Check => kernel.cachyos_check()?,
                CachyosCommand::Install { r#type, build, yes } => {
                    kernel.cachyos_install(&r#type, &build, yes)?
                }
                CachyosCommand::CheckCpu => kernel.cachyos_check_cpu()?,
            },
        },
        Commands::Driver { driver: cmd } => match cmd {
            DriverCommand::Status => driver.status()?,
            DriverCommand::Detect => driver.detect()?,
            DriverCommand::InstallNvidia { version, cuda, yes } => {
                driver.install_nvidia(version.as_deref(), cuda, yes)?
            }
            DriverCommand::RemoveNvidia { yes } => driver.remove_nvidia(yes)?,
            DriverCommand::ListNvidia => driver.list_nvidia()?,
            DriverCommand::CheckNvidia => driver.check_nvidia()?,
            DriverCommand::InstallCuda { yes } => driver.install_cuda(yes)?,
        },
        Commands::Gaming { gaming } => match gaming {
            GamingCommand::Install { yes } => gaming_mgr.install_meta(yes)?,
        },
        Commands::Deps { package, tree, reverse } => {
            if reverse {
                deps.show_reverse(&package)?;
            } else if tree {
                deps.show_tree(&package)?;
            } else {
                deps.show_tree(&package)?;
            }
        },
        Commands::Rollback { id, yes } => {
            if let Some(id) = id {
                rollback.rollback_by_id(id, yes)?;
            } else {
                rollback.rollback_last(yes)?;
            }
        },
        Commands::Group { group } => match group {
            GroupCommand::List => groups.list()?,
            GroupCommand::Info { group } => groups.info(&group)?,
            GroupCommand::Install { group, yes } => groups.install(&group, yes)?,
            GroupCommand::Remove { group, yes } => groups.remove(&group, yes)?,
        },
        Commands::Doctor => doctor.check()?,
        Commands::Flatpak { flatpak: cmd } => match cmd {
            FlatpakCommand::Search { query } => flatpak.search(&query)?,
            FlatpakCommand::Install { app_id, yes } => flatpak.install(&app_id, yes)?,
            FlatpakCommand::Remove { app_id, yes } => flatpak.remove(&app_id, yes)?,
            FlatpakCommand::Update { yes } => flatpak.update(yes)?,
            FlatpakCommand::List => flatpak.list()?,
            FlatpakCommand::Info { app_id } => flatpak.info(&app_id)?,
            FlatpakCommand::SetupFlathub => flatpak.setup_flathub()?,
        },
        Commands::Export { file, with_flatpak } => {
            if with_flatpak {
                export.export_with_flatpak(&file)?;
            } else {
                export.export(&file)?;
            }
        },
        Commands::Import { file, yes } => export.import(&file, yes)?,
        Commands::Repo { repo: cmd } => match cmd {
            RepoCommand::List { all } => repo.list(!all)?,
            RepoCommand::Enable { repo_id } => repo.enable(&repo_id)?,
            RepoCommand::Disable { repo_id } => repo.disable(&repo_id)?,
            RepoCommand::Add { name, url } => repo.add(&name, &url)?,
            RepoCommand::Remove { repo_id } => repo.remove(&repo_id)?,
            RepoCommand::Info { repo_id } => repo.info(&repo_id)?,
            RepoCommand::Refresh => repo.refresh()?,
        },
        Commands::Security { security: cmd } => match cmd {
            SecurityCommand::Check => security.check()?,
            SecurityCommand::List { severity } => security.list(severity.as_deref())?,
            SecurityCommand::Update { yes } => security.update(yes)?,
            SecurityCommand::Info { advisory_id } => security.info(&advisory_id)?,
            SecurityCommand::Cve { cve_id } => security.cve_check(&cve_id)?,
            SecurityCommand::Audit => security.audit()?,
        },
        Commands::Download { packages, dest, with_deps } => {
            if with_deps {
                let dest_dir = dest.as_deref().unwrap_or(".");
                download.download_with_deps(&packages, dest_dir)?;
            } else if let Some(dest_dir) = dest {
                download.download_to(&packages, &dest_dir)?;
            } else {
                download.download(&packages)?;
            }
        },
        Commands::InstallOffline { rpm_files, yes } => download.install_offline(&rpm_files, yes)?,
        Commands::Changelog { package, limit } => {
            if let Some(n) = limit {
                changelog.show_recent(&package, n)?;
            } else {
                changelog.show(&package)?;
            }
        },
        Commands::Whatsnew => changelog.whatsnew()?,
        Commands::Size { top, total, analyze } => {
            if analyze {
                diskspace.analyze()?;
            } else if total {
                diskspace.total_size()?;
            } else if let Some(n) = top {
                diskspace.top_packages(n)?;
            } else {
                diskspace.analyze()?;
            }
        },
        Commands::CleanOrphans { yes } => diskspace.remove_orphans(yes)?,
    }

    Ok(())
}

