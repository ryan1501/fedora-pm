#!/bin/bash

# Test script for macro expansion issue
echo "Testing macro expansion with different arguments..."

# Test version argument
echo "Test: fedora-pm self-update --version"

# Try help command  
echo "Test: fedora-pm self-update --help"

# Try unknown command
echo "Test: fedora-pm self-update unknown-command"

# Try version with verbose
echo "Test: fedora-pm self-update --version --verbose"

# Try version with force and quiet
echo "Test: fedora-pm self-update --force --quiet"