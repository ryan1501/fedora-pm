mod config;
mod driver;
mod history;
mod kernel;
mod package;
mod runner;
mod gaming;

use clap::{Parser, Subcommand};
use std::path::PathBuf;

use crate::{
    config::Config,
    driver::DriverManager,
    gaming::GamingManager,
    history::History,
    kernel::KernelManager,
    package::PackageManager,
};

#[derive(Parser, Debug)]
#[command(name = "fedora-pm", about = "Fedora Package Manager (Rust)")]
struct Cli {
    /// Path to configuration directory (defaults to ~/.fedora-pm)
    #[arg(long)]
    config_dir: Option<PathBuf>,

    /// Run package operations without sudo (for debugging)
    #[arg(long, default_value_t = true, action = clap::ArgAction::Set, value_name = "BOOL")]
    sudo: bool,

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

    let app = App::new(cli.config_dir, cli.sudo)?;
    let pkg = PackageManager::new(app.use_sudo, app.history.clone());
    let kernel = KernelManager::new(app.use_sudo, app.history.clone());
    let driver = DriverManager::new(app.use_sudo, app.history.clone());
    let gaming_mgr = GamingManager::new(app.use_sudo, app.history.clone());

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
    }

    Ok(())
}

