#!/usr/bin/env bash
# C++20 / Qt 6 installer for Fedora Package Manager.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX="/usr/local"
BUILD_DIR="$SCRIPT_DIR/build"
DRY_RUN=0
INSTALL_CLI=0
INSTALL_GUI=0
INSTALL_BOTH=0

usage() {
    cat <<'EOF'
Fedora Package Manager C++ installer

Usage: ./install.sh [OPTIONS]

Components:
  --cli                 Build and install the CLI only (default)
  --gui                 Build and install the Qt 6 GUI only
  --both                Build and install both CLI and GUI

Paths and build:
  --prefix DIR          Installation prefix (default: /usr/local)
  --user                Install under ~/.local
  --build-dir DIR       Use DIR for the CMake build tree (default: ./build)
  --build               Accepted for compatibility; this installer always builds
  --dry-run             Print planned commands without building or installing
  -h, --help            Show this help

This script builds the C++20/CMake implementation. It does not install the
legacy Rust/PySide6 implementation or install prebuilt RPMs.
Fedora build dependencies: cmake gcc-c++ qt6-qtbase-devel (Qt needed for --gui/--both)
EOF
}

while (($#)); do
    case "$1" in
        --cli) INSTALL_CLI=1; shift ;;
        --gui) INSTALL_GUI=1; shift ;;
        --both) INSTALL_BOTH=1; shift ;;
        --prefix)
            if (($# < 2)) || [[ -z "$2" ]]; then echo "--prefix requires a directory" >&2; exit 2; fi
            PREFIX="$2"; shift 2 ;;
        --user) PREFIX="$HOME/.local"; shift ;;
        --build-dir)
            if (($# < 2)) || [[ -z "$2" ]]; then echo "--build-dir requires a directory" >&2; exit 2; fi
            BUILD_DIR="$2"; shift 2 ;;
        --build) shift ;;
        --dry-run) DRY_RUN=1; shift ;;
        --rpm-install)
            echo "--rpm-install is retired; use the CMake build or the experimental RPM specs." >&2
            exit 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
    esac
done

if (( INSTALL_BOTH )); then
    INSTALL_CLI=1
    INSTALL_GUI=1
fi
if (( ! INSTALL_CLI && ! INSTALL_GUI )); then INSTALL_CLI=1; fi
if (( INSTALL_CLI && INSTALL_GUI )); then INSTALL_BOTH=1; fi

if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake is required. On Fedora: sudo dnf install cmake gcc-c++" >&2
    exit 1
fi
if ! command -v c++ >/dev/null 2>&1; then
    echo "A C++ compiler is required. On Fedora: sudo dnf install gcc-c++" >&2
    exit 1
fi

GUI_OPTION=OFF
if (( INSTALL_GUI )); then GUI_OPTION=ON; fi

if (( DRY_RUN )); then
    printf 'Build directory: %s\nInstall prefix: %s\nCLI: %s\nGUI: %s\n' "$BUILD_DIR" "$PREFIX" "$INSTALL_CLI" "$INSTALL_GUI"
    printf 'Would run: cmake -S "%s" -B "%s" -DFEDORAPM_BUILD_GUI=%s -DFEDORAPM_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Release\n' "$SCRIPT_DIR" "$BUILD_DIR" "$GUI_OPTION"
    printf 'Would run: cmake --build "%s" --parallel\nctest --test-dir "%s" --output-on-failure\n' "$BUILD_DIR" "$BUILD_DIR"
    if (( INSTALL_BOTH )); then
        printf 'Would install CLI, GUI, desktop entry, and AppStream metadata under "%s"\n' "$PREFIX"
    elif (( INSTALL_GUI )); then
        printf 'Would install GUI, desktop entry, and AppStream metadata under "%s"\n' "$PREFIX"
    else
        printf 'Would install CLI under "%s"\n' "$PREFIX"
    fi
    exit 0
fi

cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" \
    -DFEDORAPM_BUILD_GUI="$GUI_OPTION" \
    -DFEDORAPM_BUILD_TESTS=ON \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure

if (( INSTALL_GUI )) && [[ ! -x "$BUILD_DIR/fedora-pm-gui" ]]; then
    echo "Qt 6 Widgets was not found, so the GUI target was not built. Install qt6-qtbase-devel and retry." >&2
    exit 1
fi

run_install() {
    if [[ $(id -u) -eq 0 ]]; then
        "$@"
    elif [[ -d "$PREFIX" && -w "$PREFIX" ]] || [[ ! -e "$PREFIX" && -w "$(dirname "$PREFIX")" ]]; then
        "$@"
    else
        sudo "$@"
    fi
}

if (( INSTALL_BOTH )); then
    run_install cmake --install "$BUILD_DIR" --prefix "$PREFIX"
elif (( INSTALL_CLI )); then
    run_install cmake --install "$BUILD_DIR" --prefix "$PREFIX"
else
    run_install install -D -m 0755 "$BUILD_DIR/fedora-pm-gui" "$PREFIX/bin/fedora-pm-gui"
fi

if (( INSTALL_GUI )); then
    run_install install -D -m 0644 "$SCRIPT_DIR/fedora-pm.desktop" "$PREFIX/share/applications/fedora-pm.desktop"
    run_install install -D -m 0644 "$SCRIPT_DIR/fedora-pm.metainfo.xml" "$PREFIX/share/metainfo/fedora-pm.metainfo.xml"
fi

echo "Installation complete."
if (( INSTALL_CLI )); then echo "CLI: $PREFIX/bin/fedora-pm"; fi
if (( INSTALL_GUI )); then echo "GUI: $PREFIX/bin/fedora-pm-gui"; fi
