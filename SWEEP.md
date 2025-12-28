# Fedora-PM Development Guide

This file contains useful commands and information for developing fedora-pm.

## Build Commands

### Check for compilation errors
```bash
cargo check
```

### Build debug version
```bash
cargo build
```

### Build release version
```bash
cargo build --release
```

### Run the binary
```bash
cargo run -- <command>
# Example: cargo run -- install vim
```

### Install locally
```bash
cargo build --release
sudo install -m 0755 target/release/fedora-pm /usr/local/bin/fedora-pm
```

## Testing

### Run all tests
```bash
cargo test
```

### Run with verbose output
```bash
cargo test -- --nocapture
```

### Check for warnings
```bash
cargo clippy
```

### Format code
```bash
cargo fmt
```

## Project Structure

```
fedora-pm/
├── src/
│   ├── main.rs           # Main entry point, CLI parsing
│   ├── config.rs         # Configuration management
│   ├── history.rs        # Operation history tracking
│   ├── runner.rs         # Command execution utilities
│   ├── package.rs        # Core package management
│   ├── kernel.rs         # Kernel management
│   ├── driver.rs         # Driver management
│   ├── gaming.rs         # Gaming meta package
│   ├── deps.rs           # Dependency visualization
│   ├── rollback.rs       # Rollback functionality
│   ├── groups.rs         # Package groups
│   ├── doctor.rs         # System health check
│   ├── flatpak.rs        # Flatpak integration
│   ├── export.rs         # Export/import packages
│   ├── repo.rs           # Repository management
│   ├── security.rs       # Security management
│   ├── download.rs       # Download & offline install
│   ├── changelog.rs      # Changelog viewer
│   └── diskspace.rs      # Disk space analysis
├── Cargo.toml            # Dependencies and metadata
├── README.md             # Main documentation
├── FEATURES.md           # Detailed feature guide
├── QUICK_REFERENCE.md    # Quick command reference
└── CHANGELOG_NEW_FEATURES.md  # New features summary
```

## Adding New Features

### 1. Create a new module file
```bash
touch src/my_feature.rs
```

### 2. Add module to main.rs
```rust
mod my_feature;
use crate::my_feature::MyFeatureManager;
```

### 3. Add command to Commands enum
```rust
#[derive(Subcommand, Debug)]
enum Commands {
    // ... existing commands
    MyFeature {
        #[command(subcommand)]
        my_feature: MyFeatureCommand,
    },
}
```

### 4. Add subcommand enum
```rust
#[derive(Subcommand, Debug)]
enum MyFeatureCommand {
    DoSomething { arg: String },
}
```

### 5. Handle command in main()
```rust
Commands::MyFeature { my_feature } => match my_feature {
    MyFeatureCommand::DoSomething { arg } => {
        let mgr = MyFeatureManager::new(app.use_sudo);
        mgr.do_something(&arg)?;
    }
}
```

## Code Style

### Naming Conventions
- Modules: `snake_case` (e.g., `disk_space.rs`)
- Structs: `PascalCase` (e.g., `DiskSpaceManager`)
- Functions: `snake_case` (e.g., `check_disk_space`)
- Constants: `SCREAMING_SNAKE_CASE` (e.g., `MAX_RETRIES`)

### Error Handling
- Use `anyhow::Result<()>` for functions that can fail
- Use `?` operator for error propagation
- Provide context with `.context()` or `.with_context()`

### Command Execution
```rust
use crate::runner::{command, run_capture, run_inherit};

// For commands that need output
let mut cmd = command("dnf", &["list", "installed"], false);
let output = run_capture(&mut cmd, "dnf list")?;

// For interactive commands
let mut cmd = command("dnf", &["install", "vim"], self.use_sudo);
run_inherit(&mut cmd, "dnf install")?;
```

### History Logging
```rust
self.history.log("action-name", &[item1, item2])?;
```

## Dependencies

### Current Dependencies
```toml
anyhow = "1.0"           # Error handling
chrono = "0.4"           # Date/time for history
clap = "4.5"             # CLI argument parsing
dirs = "5.0"             # User directories
serde = "1.0"            # Serialization
serde_json = "1.0"       # JSON for config/history
thiserror = "1.0"        # Error types
indicatif = "0.17"       # Progress bars
colored = "2.1"          # Colored output
log = "0.4"              # Logging framework
env_logger = "0.11"      # Environment logging
```

### Adding New Dependencies
1. Add to `Cargo.toml`
2. Run `cargo build` to download
3. Import in your module: `use dependency_name::*;`

## Common Patterns

### Creating a Manager Struct
```rust
pub struct MyManager {
    use_sudo: bool,
    history: History,
}

impl MyManager {
    pub fn new(use_sudo: bool, history: History) -> Self {
        Self { use_sudo, history }
    }

    pub fn do_something(&self, arg: &str) -> Result<()> {
        println!("Doing something with: {}", arg);
        // Implementation
        self.history.log("my-action", &[arg.to_string()])?;
        Ok(())
    }
}
```

### Using Colored Output
```rust
use colored::Colorize;

println!("{}", "Success!".green());
println!("{}", "Warning!".yellow());
println!("{}", "Error!".red());
println!("{}", "Info".cyan());
println!("{}", "Important".bold());
```

### Running Commands
```rust
// Without sudo
let mut cmd = command("rpm", &["-qa"], false);

// With sudo
let mut cmd = command("dnf", &["install", "vim"], self.use_sudo);

// Capture output
let output = run_capture(&mut cmd, "command description")?;

// Inherit stdio (interactive)
run_inherit(&mut cmd, "command description")?;

// Allow failure
let output = run_capture_allow_fail(&mut cmd, "command description")?;
```

## Debugging

### Enable verbose logging
```bash
fedora-pm -vv <command>
```

### Check what command would run
Add debug prints before command execution:
```rust
println!("Would run: {:?}", cmd);
```

### Test without sudo
```bash
fedora-pm --sudo=false <command>
```

## Release Checklist

- [ ] Run `cargo fmt`
- [ ] Run `cargo clippy`
- [ ] Run `cargo test`
- [ ] Run `cargo build --release`
- [ ] Test major features manually
- [ ] Update version in `Cargo.toml`
- [ ] Update `README.md` if needed
- [ ] Update `CHANGELOG_NEW_FEATURES.md`
- [ ] Build RPM package
- [ ] Test RPM installation

## Useful Commands

### Find TODO comments
```bash
grep -r "TODO" src/
```

### Count lines of code
```bash
find src/ -name "*.rs" | xargs wc -l
```

### Check binary size
```bash
ls -lh target/release/fedora-pm
```

### Strip binary (reduce size)
```bash
strip target/release/fedora-pm
```

## Git Workflow

### Commit messages
```
feat: Add new feature
fix: Fix bug in feature
docs: Update documentation
refactor: Refactor code
test: Add tests
chore: Update dependencies
```

### Before committing
```bash
cargo fmt
cargo clippy
cargo test
```

### Pushing changes

#### Check status
```bash
git status
git log --oneline -5
```

#### Add and commit changes
```bash
git add .
git commit -m "feat: Add comprehensive package management features"
```

#### Push to remote
```bash
# Normal push
git push origin main

# Force push (use with caution - only if you own the repo)
git push origin main --force

# Push new branch
git checkout -b feature/my-feature
git push origin feature/my-feature
```

**Note:** Git push commands may require authentication (SSH passphrase or username/password).
If using automated tools, the push command may timeout waiting for credentials. In such cases,
manually run the push command in your terminal to provide authentication when prompted.

**Authentication Setup:**
- For SSH: Ensure SSH keys are set up and added to ssh-agent
- For HTTPS: Configure git credential helper to cache credentials
- Check authentication: `git remote -v` to see which protocol is being used

#### Update from remote
```bash
# Pull latest changes
git pull origin main

# Pull with rebase
git pull origin main --rebase
```

## Performance Tips

1. Use `--release` for production builds
2. Minimize allocations in hot paths
3. Use `&str` instead of `String` when possible
4. Cache expensive operations
5. Use `run_capture` only when output is needed

## Security Considerations

1. Always validate user input
2. Use `use_sudo` flag appropriately
3. Don't expose sensitive information in logs
4. Sanitize file paths
5. Check permissions before operations

## Common Issues

### "Permission denied"
- Ensure `use_sudo` is true for privileged operations
- Check file permissions

### "Command not found"
- Verify the command exists on the system
- Check PATH environment variable

### "Package not found"
- Ensure repositories are enabled
- Run `dnf makecache` to refresh metadata

## Resources

- [Rust Book](https://doc.rust-lang.org/book/)
- [Clap Documentation](https://docs.rs/clap/)
- [Anyhow Documentation](https://docs.rs/anyhow/)
- [DNF Documentation](https://dnf.readthedocs.io/)
- [RPM Documentation](https://rpm.org/documentation.html)

## Contact

For questions or contributions, see the main README.md file.
