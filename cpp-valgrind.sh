#!/bin/bash

# cpp-valgrind.sh
# Simple script to run C++ exercises with Valgrind in Podman

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Function to start Podman
start_podman() {
    echo -e "${BLUE}Starting Podman machine...${NC}"
    if ! podman machine list | grep -q "Currently running"; then
        podman machine start
        echo -e "${GREEN}Podman started.${NC}"
    else
        echo -e "${YELLOW}Podman already running.${NC}"
    fi
}

# Function to stop Podman
stop_podman() {
    echo -e "${BLUE}Stopping Podman machine...${NC}"
    podman machine stop
    echo -e "${GREEN}Podman stopped.${NC}"
}

# Function to run program normally
run_normal() {
    echo -e "${BLUE}Running program normally...${NC}"
    podman run --rm -it \
        -v "$(pwd):/workspace" \
        -w /workspace \
        gcc:latest \
        bash -c "make re && ./$EXEC_NAME"
}

# Function to run with Valgrind
run_valgrind() {
    echo -e "${BLUE}Running with Valgrind...${NC}"
    podman run --rm -it \
        -v "$(pwd):/workspace" \
        -w /workspace \
        gcc:latest \
        bash -c "apt-get update -qq && apt-get install -y -qq valgrind && make re && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$EXEC_NAME"
}

# Show usage
show_usage() {
    echo -e "${BLUE}Usage:${NC} $0 [COMMAND] [EXECUTABLE_NAME]"
    echo
    echo "Commands:"
    echo "  run [name]     - Run executable normally (default: uses NAME from Makefile)"
    echo "  valgrind [name]- Run with Valgrind"
    echo "  start          - Start Podman machine"
    echo "  stop           - Stop Podman machine"
    echo "  status         - Check Podman status"
    echo "  help           - Show this help"
    echo
    echo "Examples:"
    echo "  $0 run zombie"
    echo "  $0 valgrind zombie"
    echo "  $0 stop"
}

# Get executable name from Makefile or argument
get_exec_name() {
    if [ -n "$1" ]; then
        echo "$1"
    elif [ -f "Makefile" ]; then
        grep -E "^NAME\s*=" Makefile | head -1 | cut -d'=' -f2 | tr -d ' '
    else
        echo "a.out"
    fi
}

# Check Podman status
check_status() {
    echo -e "${BLUE}Podman machine status:${NC}"
    podman machine list
}

# Main
case "$1" in
    start)
        start_podman
        ;;
    stop)
        stop_podman
        ;;
    status)
        check_status
        ;;
    run)
        EXEC_NAME=$(get_exec_name "$2")
        echo -e "${BLUE}Executable: $EXEC_NAME${NC}"
        start_podman
        run_normal
        ;;
    valgrind)
        EXEC_NAME=$(get_exec_name "$2")
        echo -e "${BLUE}Executable: $EXEC_NAME${NC}"
        start_podman
        run_valgrind
        ;;
    help|--help|-h)
        show_usage
        ;;
    *)
        EXEC_NAME=$(get_exec_name "$1")
        echo -e "${BLUE}Executable: $EXEC_NAME${NC}"
        echo -e "${BLUE}How would you like to run?${NC}"
        echo "1) Normal mode"
        echo "2) Valgrind"
        echo "3) Just start Podman"
        echo "4) Stop Podman"
        read -p "Enter choice (1-4): " choice

        case $choice in
            1)
                start_podman
                run_normal
                ;;
            2)
                start_podman
                run_valgrind
                ;;
            3)
                start_podman
                ;;
            4)
                stop_podman
                ;;
            *)
                echo -e "${RED}Invalid choice.${NC}"
                show_usage
                ;;
        esac
        ;;
esac
