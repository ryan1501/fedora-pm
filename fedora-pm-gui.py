#!/usr/bin/env python3
"""
Fedora Package Manager - Qt GUI front-end

This GUI is a thin wrapper around the existing `fedora-pm` CLI.
It uses PySide6 (Qt for Python) and is intended to be shipped together
with the CLI so it can be installed via an RPM and launched from the
desktop menu.
"""

import subprocess
import sys
from pathlib import Path
import shutil

from PySide6.QtCore import Qt
from PySide6.QtWidgets import (
    QApplication,
    QWidget,
    QVBoxLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QPushButton,
    QTextEdit,
    QCheckBox,
    QMessageBox,
    QComboBox,
)


class FedoraPmGui(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Fedora Package Manager")
        self.resize(800, 500)

        main_layout = QVBoxLayout(self)

        # Command selection
        cmd_layout = QHBoxLayout()
        cmd_layout.addWidget(QLabel("Command:"))

        self.command_box = QComboBox()
        self.command_box.addItems(
            [
                "install",
                "remove",
                "update",
                "search",
                "info",
                "list installed",
                "list available",
                "clean",
            ]
        )
        cmd_layout.addWidget(self.command_box)

        self.yes_checkbox = QCheckBox("Auto-confirm (-y)")
        cmd_layout.addWidget(self.yes_checkbox)
        cmd_layout.addStretch()

        main_layout.addLayout(cmd_layout)

        # Package / argument input
        pkg_layout = QHBoxLayout()
        pkg_layout.addWidget(QLabel("Arguments:"))
        self.input_edit = QLineEdit()
        self.input_edit.setPlaceholderText(
            "Package names or other arguments (e.g. vim git or --no-cache)"
        )
        pkg_layout.addWidget(self.input_edit)
        main_layout.addLayout(pkg_layout)

        # Run button
        run_layout = QHBoxLayout()
        self.run_button = QPushButton("Run")
        self.run_button.clicked.connect(self.run_command)
        run_layout.addStretch()
        run_layout.addWidget(self.run_button)
        main_layout.addLayout(run_layout)

        # Output area
        main_layout.addWidget(QLabel("Output:"))
        self.output = QTextEdit()
        self.output.setReadOnly(True)
        main_layout.addWidget(self.output)

    def append_output(self, text: str):
        self.output.append(text)
        self.output.moveCursor(self.output.textCursor().End)

    def _build_cli_command(self) -> list[str]:
        """Build the fedora-pm CLI command based on UI state."""
        raw_cmd = self.command_box.currentText()
        parts = raw_cmd.split()
        command = parts[0]
        extra = parts[1:]

        args = [command] + extra

        # Extra arguments from user
        user_args = self.input_edit.text().strip()
        if user_args:
            args.extend(user_args.split())

        if self.yes_checkbox.isChecked() and command in {"install", "remove", "update"}:
            args.append("-y")

        # Prefer system-installed `fedora-pm` if available
        exe = "fedora-pm"

        # If running from source tree without installed CLI, fall back to local script
        script_path = Path(__file__).parent / "fedora-pm.py"
        if not shutil.which(exe) and script_path.exists():
            return [sys.executable, str(script_path), *args]

        return [exe, *args]

    def run_command(self):
        cmd = self._build_cli_command()
        self.append_output(f"$ {' '.join(cmd)}")

        try:
            result = subprocess.run(
                cmd,
                text=True,
                capture_output=True,
            )
        except FileNotFoundError:
            QMessageBox.critical(
                self,
                "Error",
                "Could not find `fedora-pm` executable.\n\n"
                "Make sure it is installed on your system or run the GUI from "
                "the project directory where `fedora-pm.py` is located.",
            )
            return

        if result.stdout:
            self.append_output(result.stdout.strip())
        if result.stderr:
            self.append_output(result.stderr.strip())

        if result.returncode != 0:
            self.append_output(f"[exit status: {result.returncode}]")


def main():
    app = QApplication(sys.argv)
    win = FedoraPmGui()
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()


