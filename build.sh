#!/bin/bash

# ESP32-C6 Tuya Project Build Script for Linux/macOS

# Set default port
PORT=${1:-/dev/ttyUSB0}

echo "======================================"
echo "TUYA ESP32-C6 Build Script"
echo "======================================"
echo "Target Port: $PORT"
echo ""

# Check if IDF_PATH is set
if [ -z "$IDF_PATH" ]; then
    echo "ERROR: IDF_PATH environment variable is not set"
    echo "Please run: export IDF_PATH=~/esp/esp-idf"
    exit 1
fi

# Source ESP-IDF environment
source $IDF_PATH/export.sh

# Menu options
PS3='Please select an option: '
options=("Build" "Flash" "Monitor" "Build & Flash" "Build & Flash & Monitor" "Clean" "Menu Config" "Exit")
select opt in "${options[@]}"
do
    case $opt in
        "Build")
            echo "Building project..."
            idf.py build
            ;;
        "Flash")
            echo "Flashing to device..."
            idf.py -p $PORT flash
            ;;
        "Monitor")
            echo "Monitoring serial output..."
            idf.py -p $PORT monitor
            ;;
        "Build & Flash")
            echo "Building and flashing..."
            idf.py -p $PORT build flash
            ;;
        "Build & Flash & Monitor")
            echo "Building, flashing and monitoring..."
            idf.py -p $PORT build flash monitor
            ;;
        "Clean")
            echo "Cleaning build files..."
            idf.py fullclean
            ;;
        "Menu Config")
            echo "Opening menuconfig..."
            idf.py menuconfig
            ;;
        "Exit")
            echo "Exiting..."
            break
            ;;
        *) echo "invalid option $REPLY";;
    esac
done
