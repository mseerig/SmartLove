#!/bin/bash

# SmartLove Build Script
# Compatible with ESP-IDF 4.x and 5.x

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored messages
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if ESP-IDF is set up
if [ -z "$IDF_PATH" ]; then
    print_error "ESP-IDF is not set up. Please run:"
    echo "  . ~/esp/esp-idf/export.sh"
    echo "  or"
    echo "  . \$HOME/esp/v5.3/esp-idf/export.sh"
    exit 1
fi

print_info "ESP-IDF Path: $IDF_PATH"

# Get IDF version
IDF_VERSION=$(python -c "import sys; sys.path.append('$IDF_PATH/tools'); from idf_py_actions.constants import IDF_VERSION; print(IDF_VERSION)" 2>/dev/null || echo "Unknown")
print_info "ESP-IDF Version: $IDF_VERSION"

# Parse command line arguments
COMMAND=${1:-build}
PORT=${2:-/dev/ttyUSB0}

case $COMMAND in
    clean)
        print_info "Cleaning build directory..."
        rm -rf build
        rm -f sdkconfig
        print_info "Clean complete!"
        ;;
        
    build)
        print_info "Building SmartLove..."
        idf.py build
        print_info "Build complete!"
        ;;
        
    flash)
        print_info "Flashing to device on port $PORT..."
        idf.py -p "$PORT" flash
        print_info "Flash complete!"
        ;;
        
    monitor)
        print_info "Starting serial monitor on port $PORT..."
        idf.py -p "$PORT" monitor
        ;;
        
    flash-monitor)
        print_info "Flashing and monitoring on port $PORT..."
        idf.py -p "$PORT" flash monitor
        ;;
        
    menuconfig)
        print_info "Opening menuconfig..."
        idf.py menuconfig
        ;;
        
    size)
        print_info "Analyzing binary size..."
        idf.py size
        ;;
        
    fullclean)
        print_info "Full clean (including sdkconfig)..."
        rm -rf build
        rm -f sdkconfig sdkconfig.old
        print_info "Full clean complete!"
        ;;
        
    reconfigure)
        print_info "Reconfiguring project..."
        rm -rf build
        rm -f sdkconfig sdkconfig.old
        idf.py set-target esp32
        idf.py build
        print_info "Reconfigure complete!"
        ;;
        
    help|--help|-h)
        echo "SmartLove Build Script"
        echo ""
        echo "Usage: $0 <command> [port]"
        echo ""
        echo "Commands:"
        echo "  clean          - Clean build directory"
        echo "  build          - Build the project (default)"
        echo "  flash [port]   - Flash to device"
        echo "  monitor [port] - Start serial monitor"
        echo "  flash-monitor  - Flash and monitor"
        echo "  menuconfig     - Open configuration menu"
        echo "  size           - Show binary size analysis"
        echo "  fullclean      - Remove all build files and config"
        echo "  reconfigure    - Clean and rebuild configuration"
        echo "  help           - Show this help message"
        echo ""
        echo "Examples:"
        echo "  $0 build"
        echo "  $0 flash /dev/ttyUSB0"
        echo "  $0 flash-monitor /dev/cu.usbserial-0001"
        ;;
        
    *)
        print_error "Unknown command: $COMMAND"
        echo "Run '$0 help' for usage information"
        exit 1
        ;;
esac
