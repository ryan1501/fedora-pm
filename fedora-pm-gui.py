#!/usr/bin/env python3
"""
Fedora Package Manager - Qt GUI front-end

This GUI is a thin wrapper around the existing `fedora-pm` CLI.
It uses PySide6 (Qt for Python) and is intended to be shipped together
with the CLI so it can be installed via an RPM and launched from the
desktop menu.

Features:
- Package management (install, remove, update, search)
- System health check (doctor)
- Security audits
- Flatpak integration
- Backup/restore packages
- Repository management
- Disk space analysis
- Gaming meta package installation
"""

import subprocess
import sys
from pathlib import Path
import shutil

from PySide6.QtCore import Qt, QThread, Signal
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
    QTabWidget,
    QFileDialog,
    QScrollArea,
)
from PySide6.QtGui import QFont, QTextCursor


class FedoraPmGui(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Fedora Package Manager")
        self.resize(1000, 700)

        # Apply modern styling
        self._apply_styles()

        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(10)
        main_layout.setContentsMargins(15, 15, 15, 15)

        # Create tab widget for different feature categories
        self.tabs = QTabWidget()
        self.tabs.setObjectName("mainTabs")

        # Create tabs
        self.packages_tab = self._create_packages_tab()
        self.system_tab = self._create_system_tab()
        self.flatpak_tab = self._create_flatpak_tab()
        self.advanced_tab = self._create_advanced_tab()

        self.tabs.addTab(self.packages_tab, "📦 Packages")
        self.tabs.addTab(self.system_tab, "🔧 System")
        self.tabs.addTab(self.flatpak_tab, "📱 Flatpak")
        self.tabs.addTab(self.advanced_tab, "⚙️ Advanced")

        main_layout.addWidget(self.tabs)

        # Output area (shared across all tabs)
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

    def _create_packages_tab(self):
        """Create the packages management tab."""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(15)
        layout.setContentsMargins(10, 10, 10, 10)

        # Command selection group
        cmd_group = QGroupBox("Package Operations")
        cmd_group.setObjectName("commandGroup")
        cmd_layout = QVBoxLayout(cmd_group)
        cmd_layout.setSpacing(10)

        # Command selection row
        cmd_row = QHBoxLayout()
        cmd_label = QLabel("Command:")
        cmd_label.setObjectName("sectionLabel")
        cmd_row.addWidget(cmd_label)

        self.command_box = QComboBox()
        self.command_box.setObjectName("commandCombo")
        self.command_box.addItems([
            "install",
            "remove",
            "update",
            "search",
            "info",
            "list installed",
            "list available",
            "clean",
            "history",
        ])
        cmd_row.addWidget(self.command_box)

        self.yes_checkbox = QCheckBox("Auto-confirm (-y)")
        self.yes_checkbox.setObjectName("yesCheckbox")
        cmd_row.addWidget(self.yes_checkbox)
        cmd_row.addStretch()
        cmd_layout.addLayout(cmd_row)

        # Package input
        self.input_edit = QLineEdit()
        self.input_edit.setObjectName("inputEdit")
        self.input_edit.setPlaceholderText("Package names or arguments (e.g., vim git)")
        cmd_layout.addWidget(self.input_edit)

        # Run button
        run_layout = QHBoxLayout()
        self.run_button = QPushButton("▶ Run Command")
        self.run_button.setObjectName("runButton")
        self.run_button.clicked.connect(self.run_command)
        self.run_button.setMinimumHeight(40)
        run_layout.addStretch()
        run_layout.addWidget(self.run_button)
        run_layout.addStretch()
        cmd_layout.addLayout(run_layout)

        layout.addWidget(cmd_group)

        # Quick actions
        quick_group = QGroupBox("Quick Actions")
        quick_group.setObjectName("quickGroup")
        quick_layout = QVBoxLayout(quick_group)
        quick_layout.setSpacing(10)

        # Rollback button
        rollback_btn = QPushButton("⏮️ Rollback Last Operation")
        rollback_btn.setObjectName("actionButton")
        rollback_btn.clicked.connect(lambda: self.run_cli_command(["rollback"]))
        rollback_btn.setMinimumHeight(35)
        quick_layout.addWidget(rollback_btn)

        # Export/Import buttons
        export_import_layout = QHBoxLayout()
        export_btn = QPushButton("💾 Export Packages")
        export_btn.setObjectName("actionButton")
        export_btn.clicked.connect(self.export_packages)
        export_btn.setMinimumHeight(35)
        export_import_layout.addWidget(export_btn)

        import_btn = QPushButton("📥 Import Packages")
        import_btn.setObjectName("actionButton")
        import_btn.clicked.connect(self.import_packages)
        import_btn.setMinimumHeight(35)
        export_import_layout.addWidget(import_btn)
        quick_layout.addLayout(export_import_layout)

        layout.addWidget(quick_group)
        layout.addStretch()

        return tab

    def _create_system_tab(self):
        """Create the system management tab."""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(15)
        layout.setContentsMargins(10, 10, 10, 10)

        # System health group
        health_group = QGroupBox("System Health & Maintenance")
        health_group.setObjectName("healthGroup")
        health_layout = QVBoxLayout(health_group)
        health_layout.setSpacing(10)

        # Doctor button
        doctor_btn = QPushButton("🏥 Run System Health Check (Doctor)")
        doctor_btn.setObjectName("doctorButton")
        doctor_btn.clicked.connect(lambda: self.run_cli_command(["doctor"]))
        doctor_btn.setMinimumHeight(45)
        health_layout.addWidget(doctor_btn)

        # Disk space analysis
        disk_btn = QPushButton("💾 Analyze Disk Space")
        disk_btn.setObjectName("actionButton")
        disk_btn.clicked.connect(lambda: self.run_cli_command(["size", "--analyze"]))
        disk_btn.setMinimumHeight(35)
        health_layout.addWidget(disk_btn)

        # Clean orphans
        orphans_btn = QPushButton("🗑️ Clean Orphaned Packages")
        orphans_btn.setObjectName("actionButton")
        orphans_btn.clicked.connect(lambda: self.run_cli_command(["clean-orphans", "-y"]))
        orphans_btn.setMinimumHeight(35)
        health_layout.addWidget(orphans_btn)

        layout.addWidget(health_group)

        # Security group
        security_group = QGroupBox("Security Management")
        security_group.setObjectName("securityGroup")
        security_layout = QVBoxLayout(security_group)
        security_layout.setSpacing(10)

        # Security audit
        audit_btn = QPushButton("🔒 Run Security Audit")
        audit_btn.setObjectName("securityButton")
        audit_btn.clicked.connect(lambda: self.run_cli_command(["security", "audit"]))
        audit_btn.setMinimumHeight(45)
        security_layout.addWidget(audit_btn)

        # Security updates
        sec_update_btn = QPushButton("🛡️ Install Security Updates")
        sec_update_btn.setObjectName("actionButton")
        sec_update_btn.clicked.connect(lambda: self.run_cli_command(["security", "update", "-y"]))
        sec_update_btn.setMinimumHeight(35)
        security_layout.addWidget(sec_update_btn)

        layout.addWidget(security_group)

        # Gaming meta package
        gaming_group = QGroupBox("Gaming Setup")
        gaming_group.setObjectName("gamingGroup")
        gaming_layout = QVBoxLayout(gaming_group)
        gaming_layout.setSpacing(10)

        gaming_desc = QLabel(
            "Install Fedora Gaming Meta - A comprehensive gaming setup including "
            "Steam, Lutris, Wine, GameMode, MangoHud, DXVK, and more."
        )
        gaming_desc.setWordWrap(True)
        gaming_desc.setObjectName("gamingDesc")
        gaming_layout.addWidget(gaming_desc)

        self.gaming_install_button = QPushButton("🎮 Install Gaming Meta Package")
        self.gaming_install_button.setObjectName("gamingButton")
        self.gaming_install_button.clicked.connect(self.install_gaming_meta)
        self.gaming_install_button.setMinimumHeight(45)
        gaming_layout.addWidget(self.gaming_install_button)

        layout.addWidget(gaming_group)
        layout.addStretch()

        return tab

    def _create_flatpak_tab(self):
        """Create the Flatpak management tab."""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(15)
        layout.setContentsMargins(10, 10, 10, 10)

        # Setup group
        setup_group = QGroupBox("Flatpak Setup")
        setup_group.setObjectName("flatpakGroup")
        setup_layout = QVBoxLayout(setup_group)
        setup_layout.setSpacing(10)

        setup_btn = QPushButton("🔧 Setup Flathub Repository")
        setup_btn.setObjectName("flatpakButton")
        setup_btn.clicked.connect(lambda: self.run_cli_command(["flatpak", "setup-flathub"]))
        setup_btn.setMinimumHeight(40)
        setup_layout.addWidget(setup_btn)

        layout.addWidget(setup_group)

        # Flatpak operations
        ops_group = QGroupBox("Flatpak Operations")
        ops_group.setObjectName("flatpakOpsGroup")
        ops_layout = QVBoxLayout(ops_group)
        ops_layout.setSpacing(10)

        # Search/Install row
        search_layout = QHBoxLayout()
        self.flatpak_input = QLineEdit()
        self.flatpak_input.setObjectName("inputEdit")
        self.flatpak_input.setPlaceholderText("App ID (e.g., com.spotify.Client)")
        search_layout.addWidget(self.flatpak_input)

        search_btn = QPushButton("🔍 Search")
        search_btn.setObjectName("actionButton")
        search_btn.clicked.connect(self.flatpak_search)
        search_btn.setMinimumHeight(35)
        search_layout.addWidget(search_btn)

        install_btn = QPushButton("📥 Install")
        install_btn.setObjectName("actionButton")
        install_btn.clicked.connect(self.flatpak_install)
        install_btn.setMinimumHeight(35)
        search_layout.addWidget(install_btn)

        ops_layout.addLayout(search_layout)

        # List and Update buttons
        list_update_layout = QHBoxLayout()
        list_btn = QPushButton("📋 List Installed")
        list_btn.setObjectName("actionButton")
        list_btn.clicked.connect(lambda: self.run_cli_command(["flatpak", "list"]))
        list_btn.setMinimumHeight(35)
        list_update_layout.addWidget(list_btn)

        update_btn = QPushButton("🔄 Update All")
        update_btn.setObjectName("actionButton")
        update_btn.clicked.connect(lambda: self.run_cli_command(["flatpak", "update", "-y"]))
        update_btn.setMinimumHeight(35)
        list_update_layout.addWidget(update_btn)

        ops_layout.addLayout(list_update_layout)

        layout.addWidget(ops_group)
        layout.addStretch()

        return tab

    def _create_advanced_tab(self):
        """Create the advanced features tab."""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(15)
        layout.setContentsMargins(10, 10, 10, 10)

        # Repository management
        repo_group = QGroupBox("Repository Management")
        repo_group.setObjectName("repoGroup")
        repo_layout = QVBoxLayout(repo_group)
        repo_layout.setSpacing(10)

        repo_list_btn = QPushButton("📋 List Repositories")
        repo_list_btn.setObjectName("actionButton")
        repo_list_btn.clicked.connect(lambda: self.run_cli_command(["repo", "list", "--all"]))
        repo_list_btn.setMinimumHeight(35)
        repo_layout.addWidget(repo_list_btn)

        repo_refresh_btn = QPushButton("🔄 Refresh Repository Metadata")
        repo_refresh_btn.setObjectName("actionButton")
        repo_refresh_btn.clicked.connect(lambda: self.run_cli_command(["repo", "refresh"]))
        repo_refresh_btn.setMinimumHeight(35)
        repo_layout.addWidget(repo_refresh_btn)

        layout.addWidget(repo_group)

        # Information tools
        info_group = QGroupBox("Information & Analysis")
        info_group.setObjectName("infoGroup")
        info_layout = QVBoxLayout(info_group)
        info_layout.setSpacing(10)

        # Dependency analysis
        deps_layout = QHBoxLayout()
        self.deps_input = QLineEdit()
        self.deps_input.setObjectName("inputEdit")
        self.deps_input.setPlaceholderText("Package name for dependency analysis")
        deps_layout.addWidget(self.deps_input)

        deps_tree_btn = QPushButton("🌳 Dependency Tree")
        deps_tree_btn.setObjectName("actionButton")
        deps_tree_btn.clicked.connect(self.show_deps_tree)
        deps_tree_btn.setMinimumHeight(35)
        deps_layout.addWidget(deps_tree_btn)

        info_layout.addLayout(deps_layout)

        # Changelog
        changelog_btn = QPushButton("📜 What's New in Updates")
        changelog_btn.setObjectName("actionButton")
        changelog_btn.clicked.connect(lambda: self.run_cli_command(["whatsnew"]))
        changelog_btn.setMinimumHeight(35)
        info_layout.addWidget(changelog_btn)

        # Top packages
        top_btn = QPushButton("📊 Show Top 20 Largest Packages")
        top_btn.setObjectName("actionButton")
        top_btn.clicked.connect(lambda: self.run_cli_command(["size", "--top", "20"]))
        top_btn.setMinimumHeight(35)
        info_layout.addWidget(top_btn)

        layout.addWidget(info_group)

        # Download tools
        download_group = QGroupBox("Download & Offline Tools")
        download_group.setObjectName("downloadGroup")
        download_layout = QVBoxLayout(download_group)
        download_layout.setSpacing(10)

        download_desc = QLabel("Download packages for offline installation")
        download_desc.setWordWrap(True)
        download_layout.addWidget(download_desc)

        download_input_layout = QHBoxLayout()
        self.download_input = QLineEdit()
        self.download_input.setObjectName("inputEdit")
        self.download_input.setPlaceholderText("Package names to download")
        download_input_layout.addWidget(self.download_input)

        download_btn = QPushButton("⬇️ Download")
        download_btn.setObjectName("actionButton")
        download_btn.clicked.connect(self.download_packages)
        download_btn.setMinimumHeight(35)
        download_input_layout.addWidget(download_btn)

        download_layout.addLayout(download_input_layout)

        layout.addWidget(download_group)
        layout.addStretch()

        return tab

    def _apply_styles(self):
        """Apply modern Qt stylesheet for a better appearance."""
        stylesheet = """
        /* Main window background */
        QWidget {
            background-color: #f5f5f5;
            font-family: "Segoe UI", "DejaVu Sans", sans-serif;
            font-size: 10pt;
        }

        /* Tab widget */
        QTabWidget::pane {
            border: 2px solid #d0d0d0;
            border-radius: 8px;
            background-color: white;
            padding: 5px;
        }

        QTabBar::tab {
            background-color: #ecf0f1;
            border: 2px solid #bdc3c7;
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 8px 16px;
            margin-right: 2px;
            font-weight: bold;
        }

        QTabBar::tab:selected {
            background-color: white;
            border-color: #3498db;
            color: #2c3e50;
        }

        QTabBar::tab:hover {
            background-color: #d5dbdb;
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

        /* Doctor button */
        QPushButton#doctorButton {
            background-color: #e74c3c;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 30px;
            font-weight: bold;
            font-size: 11pt;
        }

        QPushButton#doctorButton:hover {
            background-color: #c0392b;
        }

        QPushButton#doctorButton:pressed {
            background-color: #a93226;
        }

        /* Security button */
        QPushButton#securityButton {
            background-color: #f39c12;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 30px;
            font-weight: bold;
            font-size: 11pt;
        }

        QPushButton#securityButton:hover {
            background-color: #e67e22;
        }

        QPushButton#securityButton:pressed {
            background-color: #d35400;
        }

        /* Flatpak button */
        QPushButton#flatpakButton {
            background-color: #3498db;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 30px;
            font-weight: bold;
            font-size: 11pt;
        }

        QPushButton#flatpakButton:hover {
            background-color: #2980b9;
        }

        QPushButton#flatpakButton:pressed {
            background-color: #21618c;
        }

        /* Action buttons */
        QPushButton#actionButton {
            background-color: #16a085;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 20px;
            font-weight: bold;
        }

        QPushButton#actionButton:hover {
            background-color: #138d75;
        }

        QPushButton#actionButton:pressed {
            background-color: #117a65;
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
        self.output.moveCursor(QTextCursor.End)

    def run_cli_command(self, args: list[str]):
        """Run a fedora-pm CLI command with the given arguments."""
        exe = "fedora-pm"
        script_path = Path(__file__).parent / "fedora-pm.py"

        if not shutil.which(exe) and script_path.exists():
            cmd = [sys.executable, str(script_path)] + args
        else:
            cmd = [exe] + args

        self.append_output(f"$ {' '.join(cmd)}")

        try:
            result = subprocess.run(cmd, text=True, capture_output=True)
        except FileNotFoundError:
            QMessageBox.critical(
                self,
                "Error",
                "Could not find `fedora-pm` executable.\n\n"
                "Make sure it is installed on your system.",
            )
            return

        if result.stdout:
            self.append_output(result.stdout.strip())
        if result.stderr:
            self.append_output(result.stderr.strip())

        if result.returncode != 0:
            self.append_output(f"[exit status: {result.returncode}]")

    def export_packages(self):
        """Export installed packages to a file."""
        file_path, _ = QFileDialog.getSaveFileName(
            self,
            "Export Packages",
            "packages.txt",
            "Text Files (*.txt);;All Files (*)"
        )

        if file_path:
            self.run_cli_command(["export", file_path, "--with-flatpak"])

    def import_packages(self):
        """Import packages from a file."""
        file_path, _ = QFileDialog.getOpenFileName(
            self,
            "Import Packages",
            "",
            "Text Files (*.txt);;All Files (*)"
        )

        if file_path:
            reply = QMessageBox.question(
                self,
                "Confirm Import",
                f"Import packages from {Path(file_path).name}?\n\n"
                "This will install all packages listed in the file.",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.No
            )

            if reply == QMessageBox.Yes:
                self.run_cli_command(["import", file_path, "-y"])

    def flatpak_search(self):
        """Search for Flatpak applications."""
        query = self.flatpak_input.text().strip()
        if query:
            self.run_cli_command(["flatpak", "search", query])
        else:
            QMessageBox.warning(self, "Input Required", "Please enter a search query.")

    def flatpak_install(self):
        """Install a Flatpak application."""
        app_id = self.flatpak_input.text().strip()
        if app_id:
            reply = QMessageBox.question(
                self,
                "Confirm Installation",
                f"Install Flatpak: {app_id}?",
                QMessageBox.Yes | QMessageBox.No,
                QMessageBox.Yes
            )

            if reply == QMessageBox.Yes:
                self.run_cli_command(["flatpak", "install", app_id, "-y"])
        else:
            QMessageBox.warning(self, "Input Required", "Please enter an app ID.")

    def show_deps_tree(self):
        """Show dependency tree for a package."""
        package = self.deps_input.text().strip()
        if package:
            self.run_cli_command(["deps", package, "--tree"])
        else:
            QMessageBox.warning(self, "Input Required", "Please enter a package name.")

    def download_packages(self):
        """Download packages for offline installation."""
        packages = self.download_input.text().strip()
        if packages:
            pkg_list = packages.split()
            self.run_cli_command(["download"] + pkg_list + ["--with-deps"])
        else:
            QMessageBox.warning(self, "Input Required", "Please enter package names.")

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
        self.append_output("=" * 60)
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
        
        if rpmfusion_enabled:
            self.append_output("✓ RPM Fusion repositories are enabled")
        else:
            self.append_output("✗ RPM Fusion repositories are not enabled")
        
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
                self.append_output("")
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
                self.append_output("")
                self.append_output("=" * 60)
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
            self.append_output("")
            self.append_output("=" * 60)
            self.append_output(f"Installing from local RPM: {rpm_path.name}")
            self._install_rpm(str(rpm_path))
        else:
            # Try to install from repository (if available) or build from spec
            self.append_output("")
            self.append_output("=" * 60)
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
        # Get Fedora release version
        release_result = subprocess.run(
            ["rpm", "-E", "%fedora"],
            capture_output=True,
            text=True,
        )
        
        if release_result.returncode != 0:
            error_msg = "Failed to determine Fedora release version."
            if release_result.stderr:
                error_msg += f"\nError: {release_result.stderr.strip()}"
            self.append_output(f"✗ {error_msg}")
            QMessageBox.critical(
                self,
                "Error",
                f"{error_msg}\n\n"
                "Cannot enable RPM Fusion repositories without knowing the Fedora version.\n"
                "Please enable them manually or check your system configuration.",
            )
            return
        
        fedora_release = release_result.stdout.strip()
        
        if not fedora_release:
            error_msg = "Fedora release version is empty. Cannot construct repository URLs."
            self.append_output(f"✗ {error_msg}")
            QMessageBox.critical(
                self,
                "Error",
                f"{error_msg}\n\n"
                "Please enable RPM Fusion repositories manually:\n"
                "sudo dnf install https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-$(rpm -E %fedora).noarch.rpm\n"
                "sudo dnf install https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-$(rpm -E %fedora).noarch.rpm",
            )
            return
        
        free_url = f"https://mirrors.rpmfusion.org/free/fedora/rpmfusion-free-release-{fedora_release}.noarch.rpm"
        nonfree_url = f"https://mirrors.rpmfusion.org/nonfree/fedora/rpmfusion-nonfree-release-{fedora_release}.noarch.rpm"
        
        # Install free repository
        self.append_output("")
        self.append_output(f"Installing RPM Fusion Free repository...")
        self.append_output(f"URL: {free_url}")
        result = subprocess.run(
            ["sudo", "dnf", "install", "-y", free_url],
            text=True,
            capture_output=True,
        )
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        if result.returncode == 0:
            self.append_output("✓ RPM Fusion Free repository enabled")
        else:
            self.append_output("✗ Failed to enable RPM Fusion Free repository")
        
        # Install nonfree repository
        self.append_output("")
        self.append_output(f"Installing RPM Fusion Nonfree repository...")
        self.append_output(f"URL: {nonfree_url}")
        result = subprocess.run(
            ["sudo", "dnf", "install", "-y", nonfree_url],
            text=True,
            capture_output=True,
        )
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        if result.returncode == 0:
            self.append_output("✓ RPM Fusion Nonfree repository enabled")
        else:
            self.append_output("✗ Failed to enable RPM Fusion Nonfree repository")
        
        self.append_output("")
        self.append_output("=" * 60)
        self.append_output("RPM Fusion setup complete!")

    def _install_rpm(self, rpm_path: str):
        """Install an RPM file using dnf."""
        install_cmd = ["sudo", "dnf", "install", "-y", rpm_path]
        self.append_output(f"$ {' '.join(install_cmd)}")
        self.append_output("")
        
        result = subprocess.run(
            install_cmd,
            text=True,
            capture_output=True,
        )
        
        if result.stdout:
            self.append_output(result.stdout)
        if result.stderr:
            self.append_output(result.stderr)
        
        self.append_output("")
        self.append_output("=" * 60)
        
        if result.returncode == 0:
            self.append_output("✓ Installation completed successfully!")
            self.append_output("")
            self.append_output("Fedora Gaming Meta is now installed.")
            self.append_output("All gaming tools (Steam, Lutris, Wine, GameMode, MangoHud, DXVK, etc.) are available.")
            QMessageBox.information(
                self,
                "Installation Complete",
                "Fedora Gaming Meta package installed successfully!\n\n"
                "All gaming tools (Steam, Lutris, Wine, GameMode, MangoHud, DXVK, etc.) are now available.\n\n"
                "You can now launch Steam, Lutris, or use Wine to play games!",
            )
        else:
            self.append_output(f"✗ Installation failed (exit code: {result.returncode})")
            QMessageBox.critical(
                self,
                "Installation Failed",
                f"Failed to install gaming meta package.\n\n"
                f"Exit code: {result.returncode}\n\n"
                f"Check the output above for details.",
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


