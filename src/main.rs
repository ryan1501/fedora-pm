use clap::{Parser, Arg, Subcommand, CommandFactory, ArgMatches};

#[derive(Parser, Debug)]
#[command(name = "fedora-pm", about = "Fedora Package Manager (Rust)", version)]
pub struct Cli {
    #[command(subcommand)]
    Version {
        #[command(subcommand)]
        version: bool,
        #[arg(short = 'v', long = "version")]
    },
    #[command(subcommand)]
    Update {
        #[command(subcommand)]
        #[command(subcommand)]
        #[arg(short = 'f', long = "force")]
        force: bool,
        #[arg(short = 'q', long = "quiet")]
    },
    #[command(subcommand)]
    Status {
        #[command(subcommand)]
        #[command(subcommand)]
    },
}

#[derive(Subcommand)]
pub enum Commands {
    #[command(subcommand)]
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
    UpdatePackages {
        packages: Vec<String>,
        #[arg(short, long)]
        yes: bool,
    },
    Search { query: String },
    Kernel {
        #[command(subcommand)]
        #[command(subcommand)]
        #[command(subcommand)]
        #[command(subcommand)]
        Install { version: Option<String>, yes: bool },
        #[command(subcommand)]
        Remove { versions: Vec<String>, yes: bool, keep_current: bool },
        #[command(subcommand)]
        RemoveOld { keep_last: usize, yes: bool },
        #[command(subcommand)]
        List {
            available: bool, 
            installed: bool,
            all: bool,
            pattern: Option<String>,
        },
        Info { package: String },
    },
    Driver {
        #[command(subcommand)]
        #[command(subcommand)]
        Status,
        Detect,
        #[command(subcommand)]
        InstallNvidia { version: Option<String>, cuda: bool, yes: bool },
        #[command(subcommand)]
        RemoveNvidia { yes: bool },
        ListNvidia,
        CheckNvidia,
    },
    Gaming {
        #[command(subcommand)]
        Install { yes: bool },
    },
    Deps {
        package: String,
        tree: bool,
        reverse: bool,
    },
    Rollback {
        #[command(subcommand)]
        id: Option<usize>,
        yes: bool,
    },
    Group {
        #[command(subcommand)]
        group: String,
        #[command(subcommand)]
        List,
        Info { group: String },
        Install { group: String, yes: bool },
        Remove { group: String, yes: bool },
    },
    Doctor {
        #[command(subcommand)]
    },
    Flatpak {
        #[command(subcommand)]
        #[command(subcommand)]
        Search { query: String },
        Install { app_id: String, yes: bool },
        Remove { app_id: String, yes: bool },
        Update { yes: bool },
        List,
        Info { app_id: String },
        SetupFlathub,
    },
    Export {
        file: String,
        #[arg(long)]
        with_flatpak: bool,
    },
        Import { 
            file: String,
            #[command(subcommand)]
            with_flatpak: bool,
        },
    },
    },
    Repo {
        #[command(subcommand)]
        #[command(subcommand)]
        List,
        Info { repo_id: String },
        #[command(subcommand)]
        Enable { repo_id: String },
        Disable { repo_id: String },
        Refresh,
        Add { name: String, url: String },
        Remove { repo_id: String },
        Info { repo_id: String },
    },
    Security {
        #[command(subcommand)]
        Security {
            #[command(subcommand)]
            Check, },            
            List { severity: Option<String> },
            Update { yes: bool },
            Info { advisory_id: String }, 
            Cve { cve_id: String },
            Audit,
        },
    },
    Download {
        packages: Vec<String>,
        #[arg(short, long)]
        dest: Option<String>,
        with_deps: bool,
    },
    InstallOffline {
        rpm_files: Vec<String>,
        yes: bool,
    },
    },
    Changelog {
        package: String,
        limit: Option<usize>,
    },
    Whatsnew,
    Size {
        #[command(subcommand)]
        top: Option<usize>,
        total: bool,
        analyze: bool,
    },
    CleanOrphans { 
        #[command(subcommand)]
        yes: bool,
    },
    SelfUpdate {
        #[command(subcommand)]
        #[command(subcommand)]
        #[command(subcommand)]
        Status,
        #[command(subcommand)]
        Update {
            #[command(subcommand)]
            force: bool,
            quiet: bool,
        },
        Enable { frequency: String },
        Disable,
    },
}

#[derive(Debug)]
pub struct Help {
    command: Option<String>,
    subcommand: Option<Commands>,
    version: Option<String>,
    interactive: bool,
    full: bool,
}

#[command(about)]
pub fn about() {
    println!("{} {}", env!("CARGO_TARGET_DIR"));
    println!("{} {}", env!("CARGO_NAME"));
    println!("Version: {}", env!("CARGO_PKG_VERSION"));
    println!("Rust Version: {}", env!("RUSTC"));
    println!();
}

pub fn print_help(cmd: &Help) {
    use std::io::Write;
    
    match cmd.subcommand {
        Some(Commands::Help(Commands::Version { version, interactive, full, .. })) => {
            if let Some(v) = version.as_ref() {
                println!("Version: {}", v);
            }
            if full {
                println!("Full help information:");
                println!("{}", cmd.get_help().unwrap_or_default());
            } else {
                println!("{}", cmd.get_help().unwrap_or_default());
            }
            return;
        }
        
        Some(subcommand) => {
            println!("{}", subcommand.get_help().unwrap_or_default());
        }
        
        None => {
            println!("{}", HELP_TEMPLATE);
        }
    }
}

const HELP_TEMPLATE: &str = r#"
    NAME
SYNOPSIS
    {subcommand} [OPTIONS]

DESCRIPTION
    {subcommand}

OPTIONS:
    -h, --help        Show this help message
    -v, --verbose     Verbose output
    -q, --quiet      Minimal output

EXAMPLES:
    {subcommand} [ARGS]

EXAMPLES:
    {subcommand} --option value

For more information on a specific command, run:
    {subcommand} --help
"#;

impl Help {
    fn print_help(&self) -> String {
        let help = match self.command.as_deref().map(|cmd| {
            Commands::Version => HELP_TEMPLATE.replace("{command}", "version")
                    .replace("{subcommand}", "version")
                    .replace("{options}", ""),
                    .replace("{help_text}", HELP_VERSION_TEXT),
            Commands::Update { HELP_TEMPLATE.replace("{command}", "update")
                    .replace("{subcommand}", "update")
                    .replace("{options}", "[--force, --quiet]")
                    .replace("{help_text}", HELP_UPDATE_TEXT),
            Commands::Status => HELP_TEMPLATE.replace("{command}", "status")
                    .replace("{subcommand}", "status")
                    .replace("{options}", ""),
                    .replace("{help_text}", HELP_STATUS_TEXT),
            Commands::Install => HELP_TEMPLATE.replace("{command}", "install")
                    .replace("{subcommand}", "install")
                    .replace("{options}", "[--yes]")
                    .replace("{help_text}", HELP_INSTALL_TEXT),
            Commands::Remove => HELP_TEMPLATE.replace("{command}", "remove")
                    .replace("{subcommand}", "remove")
                    .replace("{options}", "[--yes]")
                    .replace("{help_text}", HELP_REMOVE_TEXT),
            Commands::Search => HELP_TEMPLATE.replace("{command}", "search")
                    .replace("{subcommand}", "search")
                    .replace("{options}", "<QUERY>"),
                    .replace("{help_text}", HELP_SEARCH_TEXT),
            Commands::Kernel => HELP_TEMPLATE.replace("{command}", "kernel")
                    .replace("{subcommand}", "kernel")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_KERNEL_TEXT),
            Commands::Driver => HELP_TEMPLATE.replace("{command}", "driver")
                    .replace("{subcommand}", "driver")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_DRIVER_TEXT),
            Commands::Gaming => HELP_TEMPLATE.replace("{command}", "gaming")
                    .replace("{subcommand}", "gaming")
                    .replace("{options}", "[--yes]")
                    .replace("{help_text}", HELP_GAMING_TEXT),
            Commands::Deps => HELP_TEMPLATE.replace("{command}", "deps")
                    .replace("{subcommand}", "deps")
                    .replace("{options}", "[--tree, --reverse]")
                    .replace("{help_text}", HELP_DEPS_TEXT),
            Commands::Rollback => HELP_TEMPLATE.replace("{command}", "rollback")
                    .replace("{subcommand}", "rollback")
                    .replace("{options}", "[--id, --yes]")
                    .replace("{help_text}", HELP_ROLLBACK_TEXT),
            Commands::Group => HELP_TEMPLATE.replace("{command}", "group")
                    .replace("{subcommand}", "group")
                    .replace("{options}", "[list, info, install, remove]")
                    .replace("{help_text}", HELP_GROUP_TEXT),
            Commands::Doctor => HELP_TEMPLATE.replace("{command}", "doctor")
                    .replace("{subcommand}", "doctor")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_DOCTOR_TEXT),
            Commands::Flatpak => HELP_TEMPLATE.replace("{command}", "flatpak")
                    .replace("{subcommand}", "flatpak")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_FLATPAK_TEXT),
            Commands::Export => HELP_TEMPLATE.replace("{command}", "export")
                    .replace("{subcommand}", "export")
                    .replace("{options}", "[--with-flatpak, --without-flatpak]")
                    .replace("{help_text}", HELP_EXPORT_TEXT),
            Commands::Import => HELP_TEMPLATE.replace("{command}", "import")
                    .replace("{subcommand}", "import")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_IMPORT_TEXT),
            Commands::Repo => HELP_TEMPLATE.replace("{command}", "repo")
                    .replace("{subcommand}", "repo")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_REPO_TEXT),
            Commands::Security => HELP_TEMPLATE.replace("{command}", "security")
                    .replace("{subcommand}", "security")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_SECURITY_TEXT),
            Commands::Download => HELP_TEMPLATE.replace("{command}", "download")
                    .replace("{options}", "[--dest=<DIR>, --with-deps]")
                    .replace("{help_text}", HELP_DOWNLOAD_TEXT),
            Commands::InstallOffline => HELP_TEMPLATE.replace("{command}", "install-offline")
                    .replace("{options}", "[yes]")
                    .replace("{help_text}", HELP_INSTALL_OFFLINE_TEXT),
            Commands::Changelog => HELP_TEMPLATE.replace("{command}", "changelog")
                    .replace("{options}", "[--limit=N]")
                    .replace("{help_text}", HELP_CHANGELOG_TEXT),
            Commands::Whatsnew => HELP_TEMPLATE.replace("{command}", "whatsnew")
                    .replace("{options}", "")
                    .replace("{help_text}", },        HELP_WHATSNEW_TEXT),
            Commands::Size => HELP_TEMPLATE.replace("{command}", "size")
                    .replace("{options}", "[--top=N, --total, --analyze]")
                    .replace("{help_text}", HELP_SIZE_TEXT),
            Commands::CleanOrphans => HELP_TEMPLATE.replace("{command}", "clean-orphans")
                    .replace("{options}", "[--yes]")
                    .replace("{help_text}", CLEAN_ORPHANS_TEXT),
            Commands::SelfUpdate => HELP_TEMPLATE.replace("{command}", "self-update")
                    .replace("{subcommand}", "update")
                    .replace("{options}", "[--force, --quiet]")
                    .replace("{help_text}", HELP_SELF_UPDATE_TEXT),
            ),
        _ => HELP_TEMPLATE.replace("{command}", "help")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_GENERAL_TEXT),
    }
}