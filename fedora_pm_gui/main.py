#!/usr/bin/env python3
"""
Main entry point for Fedora Package Manager GUI
"""

import sys
from PySide6.QtWidgets import QApplication

# Import GUI class - handle both package and direct execution
try:
    from .gui import FedoraPmGui
except ImportError:
    # Fallback for direct execution
    from gui import FedoraPmGui

def main():
    """Main entry point for the GUI application."""
    app = QApplication(sys.argv)
    app.setApplicationName("Fedora Package Manager")
    
    # Optional: Set application-wide style
    # app.setStyle("Fusion")  # Modern cross-platform style
    
    win = FedoraPmGui()
    win.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()