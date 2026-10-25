#!/usr/bin/env bash
set -euo pipefail
cat >&2 <<'EOF'
This script belongs to the legacy Rust/Cargo or Python/PySide6 packaging flow.
It is disabled on the C++20/Qt 6 migration branch to prevent accidentally
building or publishing the obsolete implementation. No action was taken.

For this branch, use the CMake-based installer (./install.sh --help) and the
current fedora-pm-rpm.spec / fedora-pm-gui-rpm.spec only after the packaging
validation checklist in docs/CPP_MIGRATION_PLAN.md is complete.
EOF
exit 2
