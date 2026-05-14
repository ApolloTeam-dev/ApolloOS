#!/bin/bash

IMAGE_NAME="apolloos/builder"
DOCKERFILE="Dockerfile"

# Colors
GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
RESET="\033[0m"

show_help() {
    echo -e "${GREEN}ApolloOS Docker Build Script${RESET}"
    echo ""
    echo -e "${YELLOW}Usage:${RESET}"
    echo "  ./docker.sh                Build ApolloOS (default)"
    echo "  ./docker.sh build          Build ApolloOS (no clean)"
    echo "  ./docker.sh clean          Clean only (make clean)"
    echo "  ./docker.sh rebuild        Clean + build"
    echo "  ./docker.sh shell          Open a shell inside the Docker build environment"
    echo "  ./docker.sh --clean-image  Delete the Docker image"
    echo "  ./docker.sh help           Show this help"
    echo ""
    exit 0
}

# -----------------------------
# Determine action
# -----------------------------
if [ -z "$1" ]; then
    ACTION="build"
else
    ACTION="$1"
fi

# -----------------------------
# Help
# -----------------------------
if [[ "$ACTION" == "help" || "$ACTION" == "-h" || "$ACTION" == "--help" ]]; then
    show_help
fi

# -----------------------------
# OPTION: clean → only make clean
# -----------------------------
if [ "$ACTION" == "clean" ]; then
    echo -e "${YELLOW}Running make clean inside Docker...${RESET}"
    docker run -it --rm \
        -v "$PWD":/usr/src \
        "$IMAGE_NAME" \
        bash -c "cd /usr/src && make clean"
    exit 0
fi

# -----------------------------
# OPTION: rebuild → clean + build
# -----------------------------
if [ "$ACTION" == "rebuild" ]; then
    echo -e "${YELLOW}Running make clean inside Docker...${RESET}"
    docker run -it --rm \
        -v "$PWD":/usr/src \
        "$IMAGE_NAME" \
        bash -c "cd /usr/src && make clean"

    echo -e "${GREEN}Rebuilding ApolloOS...${RESET}"
    docker run -it --rm \
        -v "$PWD":/usr/src \
        "$IMAGE_NAME" \
        bash -c "cd /usr/src && ./rebuild_all.sh"
    exit 0
fi

# -----------------------------
# OPTION: build → build without clean
# -----------------------------
if [ "$ACTION" == "build" ]; then
    # Build image if missing
    if ! docker image inspect "$IMAGE_NAME" > /dev/null 2>&1; then
        echo -e "${YELLOW}Docker image '$IMAGE_NAME' not found. Building it now...${RESET}"
        docker build -t "$IMAGE_NAME" .
        if [ $? -ne 0 ]; then
            echo -e "${RED}❌ Failed to build Docker image. Aborting.${RESET}"
            exit 1
        fi
    fi

    echo -e "${GREEN}Building ApolloOS (no clean)...${RESET}"
    docker run -it --rm \
        -v "$PWD":/usr/src \
        "$IMAGE_NAME" \
        bash -c "cd /usr/src && ./rebuild_all.sh"
    exit 0
fi

# -----------------------------
# OPTION: shell → open interactive shell
# -----------------------------
if [ "$ACTION" == "shell" ]; then
    echo -e "${GREEN}Opening shell inside Docker environment...${RESET}"
    docker run -it --rm \
        -v "$PWD":/usr/src \
        "$IMAGE_NAME" \
        bash
    exit 0
fi

# -----------------------------
# OPTION: --clean-image → delete Docker image
# -----------------------------
if [ "$ACTION" == "--clean-image" ]; then
    echo -e "${YELLOW}Removing Docker image '$IMAGE_NAME'...${RESET}"
    docker rmi "$IMAGE_NAME"
    exit 0
fi

# -----------------------------
# Unknown option
# -----------------------------
echo -e "${RED}Unknown option: $ACTION${RESET}"
show_help
