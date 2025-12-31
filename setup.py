#!/usr/bin/env python3
"""
Setup script for Fedora Package Manager GUI
"""

from setuptools import setup, find_packages
import os

# Read the README file
def read_readme():
    readme_path = os.path.join(os.path.dirname(__file__), 'README.md')
    if os.path.exists(readme_path):
        with open(readme_path, 'r', encoding='utf-8') as f:
            return f.read()
    return "Fedora Package Manager GUI - Qt front-end for fedora-pm"

# Read requirements
def read_requirements():
    requirements_path = os.path.join(os.path.dirname(__file__), 'requirements.txt')
    requirements = []
    if os.path.exists(requirements_path):
        with open(requirements_path, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#'):
                    requirements.append(line)
    return requirements

setup(
    name="fedora-pm-gui",
    version="1.0.0",
    author="Fedora PM Maintainer",
    author_email="maintainer@fedora-pm.org",
    description="Qt GUI front-end for Fedora Package Manager",
    long_description=read_readme(),
    long_description_content_type="text/markdown",
    url="https://github.com/fedora-pm/fedora-pm",
    license="GPLv3+",
    
    # Package information
    packages=find_packages(),
    py_modules=["fedora-pm-gui"],
    
    # Python version requirement
    python_requires=">=3.8",
    
    # Dependencies
    install_requires=read_requirements(),
    
    # Entry points
    entry_points={
        'console_scripts': [
            'fedora-pm-gui=fedora_pm_gui.main:main',
        ],
        'gui_scripts': [
            'fedora-pm-gui=fedora_pm_gui.main:main',
        ],
    },
    
    # Package metadata
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: End Users/Desktop",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Topic :: System :: Software Distribution",
        "Topic :: System :: Systems Administration",
        "Topic :: Utilities :: Package Manager",
        "Environment :: X11 Applications :: Qt",
    ],
    
    # Keywords
    keywords="package manager fedora dnf rpm gui qt",
    
    # Include additional files
    include_package_data=True,
    package_data={
        '': ['*.desktop', '*.png', '*.svg', '*.metainfo.xml'],
    },
    
    # Project URLs
    project_urls={
        "Bug Reports": "https://github.com/fedora-pm/fedora-pm/issues",
        "Source": "https://github.com/fedora-pm/fedora-pm",
        "Documentation": "https://github.com/fedora-pm/fedora-pm/wiki",
    },
)