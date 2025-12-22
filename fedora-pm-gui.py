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

        # Quick Install section for gaming meta package
        quick_install_group = QGroupBox("Quick Install")
        quick_install_group.setObjectName("quickInstallGroup")
        quick_install_layout = QVBoxLayout(quick_install_group)
        quick_install_layout.setSpacing(10)
        
        gaming_desc = QLabel(
            "Install Fedora Gaming Meta - A comprehensive gaming setup including "
            "Steam, Lutris, Wine, GameMode, MangoHud, DXVK, and more."
        )
        gaming_desc.setWordWrap(True)
        gaming_desc.setObjectName("gamingDesc")
        quick_install_layout.addWidget(gaming_desc)
        
        gaming_button_layout = QHBoxLayout()
        self.gaming_install_button = QPushButton("🎮 Install Gaming Meta Package")
        self.gaming_install_button.setObjectName("gamingButton")
        self.gaming_install_button.clicked.connect(self.install_gaming_meta)
        self.gaming_install_button.setMinimumHeight(45)
        gaming_button_layout.addStretch()
        gaming_button_layout.addWidget(self.gaming_install_button)
        gaming_button_layout.addStretch()
        quick_install_layout.addLayout(gaming_button_layout)
        
        main_layout.addWidget(quick_install_group)

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
        
        /* Gaming install button */
        QPushButton#gamingButton {
            background-color: #9b59b6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 30px;
            font-weight: bold;
            font-size: 12pt;
            min-width: 250px;
        }
        
        QPushButton#gamingButton:hover {
            background-color: #8e44ad;
        }
        
        QPushButton#gamingButton:pressed {
            background-color: #7d3c98;
        }
        
        /* Gaming description */
        QLabel#gamingDesc {
            color: #34495e;
            padding: 5px;
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

    def install_gaming_meta(self):
        """Install the Fedora Gaming Meta package."""
        # Check if package is already installed
        check_result = subprocess.run(
            ["rpm", "-q", "fedora-gaming-meta"],
            capture_output=True,
            text=True,
        )
        
        if check_result.returncode == 0:
            reply = QMessageBox.question(
                self,
                "Already Installed",
                "Fedora Gaming Meta appears to be already installed.\n\n"
                "Do you want to reinstall it?",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No,
            )
            if reply == QMessageBox.No:
                return
        
        # Check for RPM Fusion repositories
        self.append_output("Checking RPM Fusion repositories...")
        rpmfusion_check = subprocess.run(
            ["dnf", "repolist", "enabled"],
            capture_output=True,
            text=True,
        )
        
        rpmfusion_enabled = False
        if rpmfusion_check.returncode == 0:
            output = rpmfusion_check.stdout.lower()
            rpmfusion_enabled = "rpmfusion" in output
        
        if not rpmfusion_enabled:
            reply = QMessageBox.warning(
                self,
                "RPM Fusion Required",
                "RPM Fusion repositories are not enabled.\n\n"
                "The gaming meta package requires RPM Fusion for Steam and other packages.\n\n"
                "Would you like to enable RPM Fusion repositories now?\n"
                "(This will require sudo access)",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.Yes,
            )
            
            if reply == QMessageBox.Yes:
                self.append_output("Enabling RPM Fusion repositories...")
                self._enable_rpmfusion()
            else:
                QMessageBox.information(
                    self,
                    "Installation Cancelled",
                    "Please enable RPM Fusion repositories first:\n\n"
                    "sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm\n"
                    "sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm",
                )
                return
        
        # Check if gaming meta package RPM exists locally
        script_dir = Path(__file__).parent
        rpm_files = list(script_dir.glob("rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm"))
        
        if not rpm_files:
            # Try to build it first
            reply = QMessageBox.question(
                self,
                "Build Required",
                "Gaming meta package RPM not found.\n\n"
                "Would you like to build it now?\n"
                "(This requires build tools and may take a moment)",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.Yes,
            )
            
            if reply == QMessageBox.Yes:
                self.append_output("Building gaming meta package...")
                build_script = script_dir / "build-gaming-meta.sh"
                if build_script.exists():
                    build_result = subprocess.run(
                        ["bash", str(build_script)],
                        capture_output=True,
                        text=True,
                    )
                    if build_result.stdout:
                        self.append_output(build_result.stdout)
                    if build_result.stderr:
                        self.append_output(build_result.stderr)
                    
                    if build_result.returncode != 0:
                        QMessageBox.critical(
                            self,
                            "Build Failed",
                            "Failed to build gaming meta package.\n\n"
                            "Please check the output for errors.",
                        )
                        return
                    
                    # Refresh RPM file list
                    rpm_files = list(script_dir.glob("rpmbuild/RPMS/noarch/fedora-gaming-meta-*.rpm"))
                else:
                    QMessageBox.warning(
                        self,
                        "Build Script Not Found",
                        "Build script not found. Please build the package manually:\n\n"
                        "./build-gaming-meta.sh",
                    )
                    return
        
        if rpm_files:
            # Install from local RPM
            rpm_path = rpm_files[0]
            self.append_output(f"Installing from local RPM: {rpm_path.name}")
            self._install_rpm(str(rpm_path))
        else:
            # Try to install from repository (if available) or build from spec
            self.append_output("Attempting to install gaming meta package...")
            
            # First, try to install directly (if available in repo)
            install_cmd = ["sudo", "dnf", "install", "-y", "fedora-gaming-meta"]
            self.append_output(f"$ {' '.join(install_cmd)}")
            
            result = subprocess.run(
                install_cmd,
                text=True,
                capture_output=True,
            )
            
            if result.stdout:
                self.append_output(result.stdout)
            if result.stderr:
                self.append_output(result.stderr)
            
            if result.returncode != 0:
                # If not in repo, try building and installing
                QMessageBox.information(
                    self,
                    "Package Not in Repository",
                    "The gaming meta package is not available in repositories.\n\n"
                    "Please build it first using:\n"
                    "./build-gaming-meta.sh\n\n"
                    "Then install the built RPM.",
                )

    def _enable_rpmfusion(self):
        """Enable RPM Fusion repositories."""
        fedora_release = subprocess.run(
            ["rpm", "-E", "%fedora"],
            capture_output=True,
            text=True,
        ).stdout.strip()
        
        free_url = f"https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-{fedora_release}.noarch.rpm"
        nonfree_url = f"https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-{fedora_release}.noarch.rpm"
        
        # Install free repository
        self.append_output(f"Installing RPM Fusion Free: {free_url}")
        result = subprocess.run(
            ["sudo", "dnf", "install", "-y", free_url],
            text=True,
            capture_output=True,
        )
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        # Install nonfree repository
        self.append_output(f"Installing RPM Fusion Nonfree: {nonfree_url}")
        result = subprocess.run(
            ["sudo", "dnf", "install", "-y", nonfree_url],
            text=True,
            capture_output=True,
        )
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        self.append_output("RPM Fusion repositories enabled!")

    def _install_rpm(self, rpm_path: str):
        """Install an RPM file using dnf."""
        install_cmd = ["sudo", "dnf", "install", "-y", rpm_path]
        self.append_output(f"$ {' '.join(install_cmd)}")
        
        result = subprocess.run(
            install_cmd,
            text=True,
            capture_output=True,
        )
        
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        if result.returncode == 0:
            QMessageBox.information(
                self,
                "Installation Complete",
                "Fedora Gaming Meta package installed successfully!\n\n"
                "All gaming tools (Steam, Lutris, Wine, etc.) are now available.",
            )
        else:
            QMessageBox.critical(
                self,
                "Installation Failed",
                f"Failed to install gaming meta package.\n\n"
                f"Exit code: {result.returncode}\n\n"
                f"Check the output for details.",
            )


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


