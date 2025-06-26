#!/bin/bash

# Run script for the Order Book project
# This script provides simple commands to build and run the project using Docker

# Function to display help message
show_help() {
    echo "Ultra-Low-Latency Market Data Analyzer"
    echo "Usage: ./run.sh [command]"
    echo ""
    echo "Commands:"
    echo "  build         - Build the Docker image"
    echo "  dev           - Start a development shell"
    echo "  python        - Run the Python example"
    echo "  cpp           - Run the C++ example"
    echo "  test          - Run the tests"
    echo "  jupyter       - Start Jupyter Notebook server"
    echo "  clean         - Remove build artifacts and Docker containers"
    echo "  help          - Show this help message"
    echo ""
}

# Check if Docker is installed
check_docker() {
    if ! command -v docker &> /dev/null; then
        echo "Error: Docker is not installed or not in PATH"
        exit 1
    fi

    if ! command -v docker-compose &> /dev/null; then
        echo "Error: Docker Compose is not installed or not in PATH"
        exit 1
    fi
}

# Build the Docker image
build() {
    echo "Building Docker image..."
    docker-compose build
}

# Start a development shell
dev() {
    echo "Starting development shell..."
    docker-compose run dev
}

# Run the Python example
run_python() {
    echo "Running Python example..."
    docker-compose run python-example
}

# Run the C++ example
run_cpp() {
    echo "Running C++ example..."
    docker-compose run cpp-example
}

# Run the tests
run_tests() {
    echo "Running tests..."
    docker-compose run tests
}

# Start Jupyter Notebook server
jupyter() {
    echo "Starting Jupyter Notebook server at http://localhost:8888"
    docker-compose up jupyter
}

# Clean build artifacts and Docker containers
clean() {
    echo "Cleaning build artifacts and Docker containers..."
    docker-compose down
    docker-compose rm -f
    
    # Also clean local build directory
    if [ -d "build" ]; then
        echo "Removing local build directory..."
        rm -rf build
    fi
    
    if [ -d "dist" ]; then
        echo "Removing local dist directory..."
        rm -rf dist
    fi
    
    if [ -d "*.egg-info" ]; then
        echo "Removing egg-info directory..."
        rm -rf *.egg-info
    fi
    
    echo "Cleanup complete"
}

# Check if Docker is installed
check_docker

# Parse command line arguments
case "$1" in
    build)
        build
        ;;
    dev)
        dev
        ;;
    python)
        run_python
        ;;
    cpp)
        run_cpp
        ;;
    test)
        run_tests
        ;;
    jupyter)
        jupyter
        ;;
    clean)
        clean
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        show_help
        ;;
esac

exit 0 
