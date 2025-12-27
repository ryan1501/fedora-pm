#!/usr/bin/env bash
# Installation script for Fedora Package Manager

set -euo pipefail
trap 'echo "Error on line $LINENO" >&2; exit 1' ERR

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_DIR="/usr/local/bin"
SCRIPT_NAME="fedora-pm"
DRY_RUN=0

usage() {
    cat <<EOF
Usage: $0 [--prefix DIR] [--user] [--dry-run] [--help]

Installs the Fedora Package Manager executable into a system or user bin directory.

Options:
  --prefix DIR   Install into DIR (defaults to /usr/local)
  --user         Install into the current user's ~/.local/bin
  --dry-run      Show actions but do not perform them
  -h, --help     Show this help message
EOF
    exit 1
}

# Parse args
while [[ ${#} -gt 0 ]]; do
    case "$1" in
        --prefix)
            if [[ -z "${2-}" ]]; then echo "--prefix requires a directory" >&2; usage; fi
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
            echo "Unknown option: $1" >&2
            usage
            ;;
    esac
done

# Prefer a built binary, then repo executable, then the Python script
if [[ -x "$SCRIPT_DIR/target/release/$SCRIPT_NAME" ]]; then
    SRC="$SCRIPT_DIR/target/release/$SCRIPT_NAME"
elif [[ -x "$SCRIPT_DIR/$SCRIPT_NAME" ]]; then
    SRC="$SCRIPT_DIR/$SCRIPT_NAME"
elif [[ -f "$SCRIPT_DIR/${SCRIPT_NAME}.py" ]]; then
    SRC="$SCRIPT_DIR/${SCRIPT_NAME}.py"
else
    echo "Error: no executable or script found to install. Build the project first (e.g. cargo build --release) or ensure ${SCRIPT_NAME} or ${SCRIPT_NAME}.py exists." >&2
    exit 2
fi

TARGET="$INSTALL_DIR/$SCRIPT_NAME"

echo "Installing $SCRIPT_NAME -> $TARGET"

# Ensure install dir exists; use sudo when necessary
MAKE_DIR_CMD=(mkdir -p "$INSTALL_DIR")
COPY_CMD=(cp "$SRC" "$TARGET")
CHMOD_CMD=(chmod +x "$TARGET")

NEED_SUDO=0
if [[ ! -d "$INSTALL_DIR" ]]; then
    # directory doesn't exist; check parent write permission
    PARENT_DIR="$(dirname "$INSTALL_DIR")"
    if [[ ! -w "$PARENT_DIR" && $(id -u) -ne 0 ]]; then
        NEED_SUDO=1
    fi
else
    if [[ ! -w "$INSTALL_DIR" && $(id -u) -ne 0 ]]; then
        NEED_SUDO=1
    fi
fi

run_cmd() {
    if [[ $DRY_RUN -eq 1 ]]; then
        echo "+ $*"
    else
        if [[ $NEED_SUDO -eq 1 ]]; then
            sudo "$@"
        else
            "$@"
        fi
    fi
}

# Create directory and copy
run_cmd "${MAKE_DIR_CMD[@]}"
run_cmd "${COPY_CMD[@]}"
run_cmd "${CHMOD_CMD[@]}"

# Post-install messages
if [[ $DRY_RUN -eq 1 ]]; then
    echo "Dry-run complete. No changes were made."
else
    echo "✓ $SCRIPT_NAME installed to $TARGET"
    if [[ "$INSTALL_DIR" == "$HOME/.local/bin" ]]; then
        echo "Ensure $HOME/.local/bin is in your PATH, e.g. add to ~/.profile or ~/.bashrc:"
        echo '  export PATH="$HOME/.local/bin:$PATH"'
    fi
    echo "Run: $SCRIPT_NAME --help"
fi
