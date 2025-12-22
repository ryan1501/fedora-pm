# GUI Customization Guide

This guide explains how to customize the appearance of the Fedora Package Manager GUI.

## Quick Customization

The GUI uses Qt stylesheets (similar to CSS) for styling. All styling is in the `_apply_styles()` method in `fedora-pm-gui.py`.

## Color Themes

### Dark Theme

Replace the stylesheet in `_apply_styles()` with:

```python
stylesheet = """
QWidget {
    background-color: #1e1e1e;
    color: #ffffff;
    font-family: "Segoe UI", "DejaVu Sans", sans-serif;
}

QGroupBox {
    border: 2px solid #444444;
    background-color: #2d2d2d;
    color: #ffffff;
}

QLineEdit, QComboBox {
    background-color: #3d3d3d;
    border: 2px solid #555555;
    color: #ffffff;
}

QPushButton#runButton {
    background-color: #0e7c0e;
    color: white;
}

QTextEdit#outputText {
    background-color: #000000;
    color: #00ff00;
}
"""
```

### Light Theme (Current)

The current theme uses light colors. You can adjust colors by changing hex values:
- `#f5f5f5` - Light gray background
- `#3498db` - Blue accents
- `#27ae60` - Green button
- `#2c3e50` - Dark text

### Fedora Blue Theme

```python
stylesheet = """
QWidget {
    background-color: #ffffff;
}

QPushButton#runButton {
    background-color: #3c6eb4;  /* Fedora blue */
    color: white;
}

QComboBox#commandCombo {
    border-color: #3c6eb4;
}

QComboBox#commandCombo:hover {
    border-color: #2d5a9e;
}
"""
```

## Changing Fonts

### Use System Font

```python
QWidget {
    font-family: system-ui, -apple-system, sans-serif;
}
```

### Use Monospace Font

```python
QWidget {
    font-family: "Fira Code", "Consolas", monospace;
}
```

### Change Font Size

```python
QWidget {
    font-size: 12pt;  /* Increase from 10pt */
}
```

## Window Size and Layout

### Change Default Window Size

In `__init__()`:
```python
self.resize(1200, 700)  # Width x Height
```

### Change Spacing

```python
main_layout.setSpacing(20)  # More space between widgets
main_layout.setContentsMargins(30, 30, 30, 30)  # More padding
```

## Button Styles

### Rounded Buttons

```python
QPushButton#runButton {
    border-radius: 20px;  /* More rounded */
}
```

### Flat Button Style

```python
QPushButton#runButton {
    background-color: transparent;
    border: 2px solid #3498db;
    color: #3498db;
}
```

### Icon Buttons

Add icons using QIcon:
```python
from PySide6.QtGui import QIcon

self.run_button.setIcon(QIcon.fromTheme("system-run"))
```

## Output Area Styling

### Terminal-like Output

```python
QTextEdit#outputText {
    background-color: #000000;
    color: #00ff00;  /* Green text */
    font-family: "Courier New", monospace;
}
```

### Syntax Highlighting Colors

You can add colored text programmatically:
```python
from PySide6.QtGui import QTextCharFormat, QColor

# In append_output method:
if "Error" in text:
    format = QTextCharFormat()
    format.setForeground(QColor("#e74c3c"))  # Red
    cursor = self.output.textCursor()
    cursor.setCharFormat(format)
    cursor.insertText(text)
```

## Using Qt Styles

Instead of custom stylesheets, you can use built-in Qt styles:

### Fusion Style (Modern Cross-platform)

```python
def main():
    app = QApplication(sys.argv)
    app.setStyle("Fusion")  # Add this line
    win = FedoraPmGui()
    win.show()
    sys.exit(app.exec())
```

### Other Built-in Styles

- `"Windows"` - Windows-style
- `"macOS"` - macOS-style (on macOS)
- `"GTK+"` - GTK-style (on Linux)

## Adding Icons

### Use System Icons

```python
from PySide6.QtGui import QIcon

self.run_button.setIcon(QIcon.fromTheme("system-run"))
self.command_box.setItemIcon(0, QIcon.fromTheme("system-software-install"))
```

### Custom Icons

```python
self.run_button.setIcon(QIcon("path/to/icon.png"))
```

## Advanced: Custom Widgets

### Add a Status Bar

```python
self.status_bar = QLabel("Ready")
self.status_bar.setObjectName("statusBar")
main_layout.addWidget(self.status_bar)
```

### Add Progress Bar

```python
from PySide6.QtWidgets import QProgressBar

self.progress = QProgressBar()
self.progress.setVisible(False)
main_layout.addWidget(self.progress)
```

## Tips

1. **Test Changes**: Run `python3 fedora-pm-gui.py` after each change
2. **Color Picker**: Use online tools like [Coolors](https://coolors.co) to pick color schemes
3. **Qt Documentation**: See [Qt Style Sheets Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
4. **Object Names**: Use `setObjectName()` to target specific widgets in stylesheets

## Example: Complete Dark Theme

See the "Dark Theme" section above for a complete dark theme stylesheet.

## Example: Minimalist Theme

```python
stylesheet = """
QWidget {
    background-color: white;
    font-size: 11pt;
}

QGroupBox {
    border: 1px solid #e0e0e0;
    border-radius: 4px;
    margin-top: 10px;
    padding-top: 10px;
}

QPushButton#runButton {
    background-color: #0078d4;
    color: white;
    border-radius: 4px;
    padding: 8px 20px;
}

QLineEdit, QComboBox {
    border: 1px solid #d0d0d0;
    border-radius: 4px;
    padding: 6px;
}
"""
```

