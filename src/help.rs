/// Show help message
        #[command(subcommand)]
        Help {
            command: Help,
        },
        short_help: &'static str = "fedora-pm [OPTIONS] [COMMAND]
        
        /// Print version information
        #[command(subcommand)]
        Version {
            version: bool,
            interactive: bool,
            first_help: bool,
            full: bool,
        },
        
        /// Print general help
        _ => {
            println!("{}", HELP_TEMPLATE.replace("{command}", "help")
                    .replace("{options}", "")
                    .replace("{help_text}", HELP_GENERAL_TEXT),
        }