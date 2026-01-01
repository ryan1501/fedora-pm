#!/bin/bash

# Unified Installation Script for Fedora Package Manager (CLI + GUI)

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/usr/local/bin"
SCRIPT_NAME="fedora-pm"
DRY_RUN=0
AUTO_BUILD=0
RPM_INSTALL=0

# Parse command line arguments
INSTALL_CLI=0
INSTALL_GUI=0
INSTALL_BOTH=0

# Help function
usage() {
    cat <<EOF
${BLUE}Fedora Package Manager - Unified Installation Script${NC}

${YELLOW}Usage:${NC} $0 [OPTIONS]

${YELLOW}Installation Options:${NC}
  --cli              Install CLI only (Rust binary)
  --gui              Install GUI only (PySide6 interface)
  --both             Install both CLI and GUI (recommended, default)

${YELLOW}Installation Locations:${NC}
  --prefix DIR       Install into DIR (default: /usr/local/bin)
  --user             Install into user directory (\$HOME/.local/bin)

${YELLOW}Build Options:${NC}
  --build            Auto-build CLI from source
  --rpm-install      Install GUI from RPM package (if available)

${YELLOW}Utility Options:${NC}
  --dry-run          Show actions without executing them
  -h, --help         Show this help message

${YELLOW}Examples:${NC}
  $0 --both                    # Install both CLI and GUI (recommended)
  $0 --cli                     # Install CLI only
  $0 --gui                     # Install GUI only
  $0 --user --both             # Install both to user directory
  $0 --prefix /opt/fedora-pm --both  # Install both to custom path
  $0 --cli --build             # Install CLI with auto-build
  $0 --gui --rpm-install       # Install GUI from RPM package
  $0 --dry-run --both          # Preview what would be installed

${YELLOW}Installation Types:${NC}
  1. CLI Only: ${GREEN}Fast, lightweight command-line tool${NC}
     - Built from source (Rust)
     - Full package management functionality
     - Ideal for servers and scripting
     
  2. GUI Only: ${GREEN}Modern graphical interface${NC}
     - Built with PySide6 (Qt)
     - User-friendly package management
     - Quick install buttons for common tasks
     
  3. Both: ${GREEN}Complete package management suite${NC}
     - CLI for power users and automation
     - GUI for everyday use and beginners
     - Seamless integration between both

${YELLOW}System Requirements:${NC}
  - Fedora Linux 38+
  - sudo access for system-wide installation
  - Rust toolchain (for CLI auto-build)
  - Python 3.8+ with PySide6 (for GUI)

${BLUE}For detailed GUI RPM installation, see GUI_RPM_INSTALLATION.md${NC}
EOF
    exit 0
}

# Print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --cli)
            INSTALL_CLI=1
            shift
            ;;
        --gui)
            INSTALL_GUI=1
            shift
            ;;
        --both)
            INSTALL_BOTH=1
            shift
            ;;
        --build)
            AUTO_BUILD=1
            shift
            ;;
        --rpm-install)
            RPM_INSTALL=1
            shift
            ;;
        --prefix)
            if [[ -z "${2-}" ]]; then 
                print_error "--prefix requires a directory"
                usage
            fi
            INSTALL_DIR="$2"
            shift 2
            ;;
        --user)
            INSTALL_DIR="$HOME/.local/bin"
            shift
            ;;
        --dry-run)
            DRY_RUN=1
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            print_error "Unknown option: $1"
            usage
            ;;
    esac
done

# Determine what to install
if [[ $INSTALL_BOTH -eq 1 ]]; then
    print_info "Installing both CLI and GUI (recommended)..."
    INSTALL_CLI=1
    INSTALL_GUI=1
elif [[ $INSTALL_CLI -eq 1 ]]; then
    print_info "Installing CLI only..."
elif [[ $INSTALL_GUI -eq 1 ]]; then
    print_info "Installing GUI only..."
else
    print_info "Installing CLI (default)..."
    INSTALL_CLI=1
fi

# Check dependencies
check_dependencies() {
    local missing_deps=()
    
    if [[ $INSTALL_CLI -eq 1 ]] && [[ $AUTO_BUILD -eq 1 ]]; then
        if ! command -v cargo &> /dev/null; then
            missing_deps+=("rustc")
            missing_deps+=("cargo")
        fi
    fi
    
    if [[ $INSTALL_GUI -eq 1 ]] && [[ $RPM_INSTALL -eq 1 ]]; then
        # Only check for Python deps if using legacy Python GUI
        if [[ -f "$SCRIPT_DIR/fedora-pm-gui.py" ]] || [[ -f "$SCRIPT_DIR/fedora_pm_gui/main.py" ]]; then
            if ! python3 -c "import PySide6" &> /dev/null 2>&1; then
                missing_deps+=("python3-pyside6")
                missing_deps+=("python3-pyside6-qtwidgets")
            fi
        fi
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_warning "Missing dependencies: ${missing_deps[*]}"
        if [[ $DRY_RUN -eq 0 ]]; then
            print_info "Installing missing dependencies..."
            sudo dnf install -y "${missing_deps[@]}" || {
                print_error "Failed to install dependencies"
                print_info "Please install manually: sudo dnf install ${missing_deps[*]}"
                exit 1
            }
        fi
    fi
}

# CLI Installation
install_cli() {
    print_info "Setting up CLI installation..."
    
    local src_binary=""
    
    if [[ $AUTO_BUILD -eq 1 ]]; then
        print_info "Auto-building CLI from source..."
        if [[ $DRY_RUN -eq 1 ]]; then
            print_info "[DRY RUN] Would build with: cargo build --release"
            src_binary="$SCRIPT_DIR/target/release/$SCRIPT_NAME"
        else
            if cargo build --release; then
                src_binary="$SCRIPT_DIR/target/release/$SCRIPT_NAME"
                print_success "CLI built successfully"
            else
                print_error "Failed to build CLI. Make sure Rust and cargo are installed."
                exit 1
            fi
        fi
    elif [[ -x "$SCRIPT_DIR/target/release/$SCRIPT_NAME" ]]; then
        src_binary="$SCRIPT_DIR/target/release/$SCRIPT_NAME"
        print_info "Found pre-built CLI binary"
    elif [[ -x "$SCRIPT_DIR/$SCRIPT_NAME" ]]; then
        src_binary="$SCRIPT_DIR/$SCRIPT_NAME"
        print_info "Found CLI binary in root"
    else
        print_error "No CLI executable found. Build the project first:"
        print_info "  ./install.sh --cli --build"
        exit 2
    fi
    
    CLI_TARGET="$INSTALL_DIR/$SCRIPT_NAME"
    MAKE_DIR_CMD=(mkdir -p "$INSTALL_DIR")
    COPY_CLI_CMD=(cp "$src_binary" "$CLI_TARGET")
    CHMOD_CLI_CMD=(chmod +x "$CLI_TARGET")
}

# GUI Installation
install_gui() {
    print_info "Setting up GUI installation..."
    
    if [[ $RPM_INSTALL -eq 1 ]]; then
        print_info "Installing GUI from RPM package..."
        # Check for GUI RPM in both x86_64 and noarch directories
        local rpm_path=""
        if compgen -G "$SCRIPT_DIR/rpmbuild/RPMS/x86_64/fedora-pm-gui-*.rpm" > /dev/null 2>&1; then
            rpm_path="$SCRIPT_DIR/rpmbuild/RPMS/x86_64/fedora-pm-gui-*.rpm"
        elif compgen -G "$SCRIPT_DIR/rpmbuild/RPMS/noarch/fedora-pm-gui-*.rpm" > /dev/null 2>&1; then
            rpm_path="$SCRIPT_DIR/rpmbuild/RPMS/noarch/fedora-pm-gui-*.rpm"
        fi
        
        if [[ -n "$rpm_path" ]]; then
            if [[ $DRY_RUN -eq 1 ]]; then
                print_info "[DRY RUN] Would install: sudo dnf install $rpm_path"
            else
                sudo dnf install "$rpm_path"
            fi
            GUI_TARGET="/usr/bin/fedora-pm-gui"
        else
            if [[ $DRY_RUN -eq 1 ]]; then
                print_info "[DRY RUN] No GUI RPM found - would prompt to build it first"
            else
                print_error "No GUI RPM found. Build it first with:"
                print_info "  cargo build --release --bin fedora-pm-gui"
                exit 2
            fi
        fi
    else
        local gui_src=""
        
        # Look for native GUI binary
        if [[ -x "$SCRIPT_DIR/target/release/fedora-pm-gui" ]]; then
            gui_src="$SCRIPT_DIR/target/release/fedora-pm-gui"
            print_info "Found native GUI binary"
        elif [[ -f "$SCRIPT_DIR/src/bin/fedora-pm-gui.rs" ]]; then
            print_info "Building GUI from source..."
            if [[ $DRY_RUN -eq 1 ]]; then
                print_info "[DRY RUN] Would build: cargo build --release --bin fedora-pm-gui"
                gui_src="$SCRIPT_DIR/target/release/fedora-pm-gui"
            else
                if cargo build --release --bin fedora-pm-gui; then
                    gui_src="$SCRIPT_DIR/target/release/fedora-pm-gui"
                    print_success "GUI built successfully"
                else
                    print_error "Failed to build GUI. Make sure Rust and cargo are installed."
                    exit 1
                fi
            fi
        elif [[ -f "$SCRIPT_DIR/fedora-pm-gui.py" ]]; then
            gui_src="$SCRIPT_DIR/fedora-pm-gui.py"
            print_warning "Found legacy Python GUI - consider using native Rust GUI"
        elif [[ -f "$SCRIPT_DIR/fedora_pm_gui/main.py" ]]; then
            gui_src="$SCRIPT_DIR/fedora_pm_gui/main.py"
            print_warning "Found legacy Python GUI - consider using native Rust GUI"
        else
            print_error "No GUI executable found. Build with: cargo build --release --bin fedora-pm-gui"
            exit 2
        fi
        
        GUI_TARGET="$INSTALL_DIR/${SCRIPT_NAME}-gui"
        COPY_GUI_CMD=(cp "$gui_src" "$GUI_TARGET")
        CHMOD_GUI_CMD=(chmod +x "$GUI_TARGET")
    fi
}

# Check if we need sudo
check_sudo_requirements() {
    NEED_SUDO=0
    
    if [[ ! -d "$INSTALL_DIR" ]]; then
        PARENT_DIR="$(dirname "$INSTALL_DIR")"
        if [[ ! -w "$PARENT_DIR" && $(id -u) -ne 0 ]]; then
            NEED_SUDO=1
        fi
    elif [[ ! -w "$INSTALL_DIR" && $(id -u) -ne 0 ]]; then
        NEED_SUDO=1
    fi
}

# Execute command (with sudo if needed)
run_cmd() {
    if [[ $DRY_RUN -eq 1 ]]; then
        if [[ $NEED_SUDO -eq 1 ]]; then
            echo "[DRY RUN] sudo $*"
        else
            echo "[DRY RUN] $*"
        fi
    else
        if [[ $NEED_SUDO -eq 1 ]]; then
            sudo "$@"
        else
            "$@"
        fi
    fi
}

# Main installation process
main() {
    print_info "Starting Fedora Package Manager installation..."
    
    if [[ $DRY_RUN -eq 1 ]]; then
        print_warning "DRY RUN MODE - No actual changes will be made"
    fi
    
    # Check dependencies
    check_dependencies
    
    # Determine installation locations and commands
    if [[ $INSTALL_CLI -eq 1 ]]; then
        install_cli
    fi
    
    if [[ $INSTALL_GUI -eq 1 ]]; then
        install_gui
    fi
    
    # Check if sudo is needed
    check_sudo_requirements
    
    # Create install directory
    run_cmd "${MAKE_DIR_CMD[@]}"
    
    # Install components
    if [[ $INSTALL_CLI -eq 1 ]]; then
        run_cmd "${COPY_CLI_CMD[@]}"
        run_cmd "${CHMOD_CLI_CMD[@]}"
    fi
    
    if [[ $INSTALL_GUI -eq 1 && $RPM_INSTALL -ne 1 ]]; then
        run_cmd "${COPY_GUI_CMD[@]}"
        run_cmd "${CHMOD_GUI_CMD[@]}"
    fi
    
    # Installation complete
    if [[ $DRY_RUN -eq 1 ]]; then
        print_success "Dry run completed. No changes were made."
        exit 0
    fi
    
    # Show installation summary
    echo
    print_success "Installation completed successfully!"
    echo
    
    if [[ $INSTALL_CLI -eq 1 && $INSTALL_GUI -eq 1 ]]; then
        echo -e "${GREEN}✓${NC} CLI installed at: ${BLUE}$CLI_TARGET${NC}"
        if [[ $RPM_INSTALL -eq 1 ]]; then
            echo -e "${GREEN}✓${NC} GUI installed via RPM package"
        else
            echo -e "${GREEN}✓${NC} GUI installed at: ${BLUE}$GUI_TARGET${NC}"
        fi
        echo
        echo -e "${YELLOW}Usage:${NC}"
        echo "  Run '${BLUE}fedora-pm${NC}' for CLI interface"
        echo "  Run '${BLUE}fedora-pm-gui${NC}' for graphical interface"
    elif [[ $INSTALL_CLI -eq 1 ]]; then
        echo -e "${GREEN}✓${NC} CLI installed at: ${BLUE}$CLI_TARGET${NC}"
        echo
        echo -e "${YELLOW}Usage:${NC}"
        echo "  Run '${BLUE}fedora-pm${NC}' for package management"
    elif [[ $INSTALL_GUI -eq 1 ]]; then
        if [[ $RPM_INSTALL -eq 1 ]]; then
            echo -e "${GREEN}✓${NC} GUI installed via RPM package"
        else
            echo -e "${GREEN}✓${NC} GUI installed at: ${BLUE}$GUI_TARGET${NC}"
        fi
        echo
        echo -e "${YELLOW}Usage:${NC}"
        echo "  Run '${BLUE}fedora-pm-gui${NC}' for graphical interface"
    fi
    
    # Path recommendations
    echo
    if [[ "$INSTALL_DIR" == "$HOME/.local/bin" ]]; then
        echo -e "${YELLOW}Note:${NC} Ensure \$HOME/.local/bin is in your PATH:"
        echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
        echo "  Add to ~/.bashrc or ~/.profile for persistence"
    elif [[ "$INSTALL_DIR" == "/usr/local/bin" ]]; then
        echo -e "${YELLOW}Note:${NC} Ensure /usr/local/bin is in your PATH"
        echo "  Most Fedora systems include this by default"
    fi
    
    # Additional information
    if [[ $INSTALL_GUI -eq 1 ]]; then
        echo
        print_info "For detailed GUI RPM installation and troubleshooting, see:"
        echo "  ${BLUE}GUI_RPM_INSTALLATION.md${NC}"
    fi
    
    echo
    print_success "Fedora Package Manager is ready to use!"
}

# Run main function
main "$@"