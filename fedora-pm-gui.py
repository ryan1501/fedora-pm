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
    QGroupBox,
)
from PySide6.QtGui import QFont, QPalette, QColor


class FedoraPmGui(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Fedora Package Manager")
        self.resize(900, 600)
        
        # Apply modern styling
        self._apply_styles()

        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(15)
        main_layout.setContentsMargins(20, 20, 20, 20)

        # Command selection group
        cmd_group = QGroupBox("Command")
        cmd_group.setObjectName("commandGroup")
        cmd_layout = QHBoxLayout(cmd_group)
        cmd_layout.setSpacing(10)

        cmd_label = QLabel("Command:")
        cmd_label.setObjectName("sectionLabel")
        cmd_layout.addWidget(cmd_label)

        self.command_box = QComboBox()
        self.command_box.setObjectName("commandCombo")
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
        self.yes_checkbox.setObjectName("yesCheckbox")
        cmd_layout.addWidget(self.yes_checkbox)
        cmd_layout.addStretch()

        main_layout.addWidget(cmd_group)

        # Package / argument input group
        input_group = QGroupBox("Arguments")
        input_group.setObjectName("inputGroup")
        pkg_layout = QVBoxLayout(input_group)
        pkg_layout.setSpacing(5)
        
        self.input_edit = QLineEdit()
        self.input_edit.setObjectName("inputEdit")
        self.input_edit.setPlaceholderText(
            "Package names or other arguments (e.g. vim git or --no-cache)"
        )
        pkg_layout.addWidget(self.input_edit)
        main_layout.addWidget(input_group)

        # Run button
        run_layout = QHBoxLayout()
        self.run_button = QPushButton("▶ Run Command")
        self.run_button.setObjectName("runButton")
        self.run_button.clicked.connect(self.run_command)
        self.run_button.setMinimumHeight(40)
        run_layout.addStretch()
        run_layout.addWidget(self.run_button)
        run_layout.addStretch()
        main_layout.addLayout(run_layout)

        # Output area group
        output_group = QGroupBox("Output")
        output_group.setObjectName("outputGroup")
        output_layout = QVBoxLayout(output_group)
        output_layout.setContentsMargins(5, 10, 5, 5)
        
        self.output = QTextEdit()
        self.output.setObjectName("outputText")
        self.output.setReadOnly(True)
        self.output.setFont(QFont("Monospace", 9))
        output_layout.addWidget(self.output)
        main_layout.addWidget(output_group)

    def _apply_styles(self):
        """Apply modern Qt stylesheet for a better appearance."""
        # You can customize colors, fonts, and styles here
        stylesheet = """
        /* Main window background */
        QWidget {
            background-color: #f5f5f5;
            font-family: "Segoe UI", "DejaVu Sans", sans-serif;
            font-size: 10pt;
        }
        
        /* Group boxes */
        QGroupBox {
            font-weight: bold;
            border: 2px solid #d0d0d0;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 15px;
            background-color: white;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            background-color: white;
            color: #2c3e50;
        }
        
        /* Labels */
        QLabel#sectionLabel {
            font-weight: bold;
            color: #2c3e50;
        }
        
        /* Combo box */
        QComboBox#commandCombo {
            padding: 6px 12px;
            border: 2px solid #3498db;
            border-radius: 6px;
            background-color: white;
            min-width: 150px;
        }
        
        QComboBox#commandCombo:hover {
            border-color: #2980b9;
            background-color: #ecf0f1;
        }
        
        QComboBox#commandCombo::drop-down {
            border: none;
            width: 20px;
        }
        
        QComboBox#commandCombo QAbstractItemView {
            border: 2px solid #3498db;
            border-radius: 6px;
            background-color: white;
            selection-background-color: #3498db;
            selection-color: white;
        }
        
        /* Line edit */
        QLineEdit#inputEdit {
            padding: 8px 12px;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            background-color: white;
            font-size: 10pt;
        }
        
        QLineEdit#inputEdit:focus {
            border-color: #3498db;
            background-color: #f8f9fa;
        }
        
        /* Push button */
        QPushButton#runButton {
            background-color: #27ae60;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 11pt;
            min-width: 150px;
        }
        
        QPushButton#runButton:hover {
            background-color: #229954;
        }
        
        QPushButton#runButton:pressed {
            background-color: #1e8449;
        }
        
        /* Checkbox */
        QCheckBox#yesCheckbox {
            color: #2c3e50;
            spacing: 5px;
        }
        
        QCheckBox#yesCheckbox::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #3498db;
            border-radius: 4px;
            background-color: white;
        }
        
        QCheckBox#yesCheckbox::indicator:checked {
            background-color: #3498db;
            border-color: #2980b9;
        }
        
        /* Text edit (output) */
        QTextEdit#outputText {
            background-color: #2c3e50;
            color: #ecf0f1;
            border: 2px solid #34495e;
            border-radius: 6px;
            padding: 10px;
            font-family: "Consolas", "Monaco", "Courier New", monospace;
        }
        
        /* Scrollbar styling */
        QScrollBar:vertical {
            background-color: #34495e;
            width: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:vertical {
            background-color: #7f8c8d;
            border-radius: 6px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background-color: #95a5a6;
        }
        
        QScrollBar:horizontal {
            background-color: #34495e;
            height: 12px;
            border-radius: 6px;
        }
        
        QScrollBar::handle:horizontal {
            background-color: #7f8c8d;
            border-radius: 6px;
            min-width: 20px;
        }
        
        QScrollBar::handle:horizontal:hover {
            background-color: #95a5a6;
        }
        """
        
        self.setStyleSheet(stylesheet)
        
        # Alternative: Use system theme (uncomment to use native OS styling)
        # app = QApplication.instance()
        # app.setStyle("Fusion")  # or "Windows", "macOS", etc.

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
    app.setApplicationName("Fedora Package Manager")
    
    # Optional: Set application-wide style
    # app.setStyle("Fusion")  # Modern cross-platform style
    
    win = FedoraPmGui()
    win.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()


