#!/usr/bin/env python3
"""
Standalone launcher for Fedora Package Manager GUI
This script can be used as the main entry point for the RPM package.
"""

import sys
import os
from pathlib import Path

# Add the package directory to Python path
package_dir = Path(__file__).parent / "fedora_pm_gui"
if package_dir.exists():
    sys.path.insert(0, str(package_dir))

# Try to import the GUI class
try:
    from gui import FedoraPmGui
except ImportError:
    print("Error: Could not import FedoraPmGui class", file=sys.stderr)
    print("Please ensure the GUI module is properly installed.", file=sys.stderr)
    sys.exit(1)

from PySide6.QtWidgets import QApplication

def main():
    """Main entry point for the GUI application."""
    app = QApplication(sys.argv)
    app.setApplicationName("Fedora Package Manager")
    
    win = FedoraPmGui()
    win.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()