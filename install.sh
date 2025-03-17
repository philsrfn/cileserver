#!/bin/bash

# Check if meson is installed
if ! command -v meson &> /dev/null; then
    echo "Meson build system not found. Please install it first:"
    echo "  pip install meson ninja"
    exit 1
fi

# Create necessary directories if they don't exist
mkdir -p config logs test_files

# Setup build directory
echo "Setting up build directory..."
meson setup builddir

# Build the project
echo "Building project..."
meson compile -C builddir

# Create config file if it doesn't exist
if [ ! -f "config/cileserver.conf" ]; then
    echo "Creating default configuration file..."
    cp config/cileserver.conf.example config/cileserver.conf
    echo "Please edit config/cileserver.conf to configure the server."
fi

echo ""
echo "Installation complete!"
echo ""
echo "To run the server: ./builddir/cileserver config/cileserver.conf"
echo "To run the client: ./builddir/cileclient"
echo ""
echo "For more information, see the README.md file." 