#!/bin/bash

echo "Building GameEngine Script with g++..."

SCRIPT_FILE="$1"
SCRIPT_NAME=$(basename "$SCRIPT_FILE" .cpp)

if [ -z "$SCRIPT_FILE" ]; then
    echo "Usage: $0 <script.cpp>"
    exit 1
fi

if [ ! -f "$SCRIPT_FILE" ]; then
    echo "Error: Script file '$SCRIPT_FILE' not found"
    exit 1
fi

echo "Compiling: $SCRIPT_FILE -> $SCRIPT_NAME.so"

g++ -shared -fPIC -std=c++20 \
    -I../src \
    -DGAMEENGINESCRIPT_EXPORTS \
    "$SCRIPT_FILE" \
    -o "$SCRIPT_NAME.so" \
    -L../build/src/Core \
    -L../build/src/Rendering \
    -lCore -lRendering -ldl

if [ $? -eq 0 ]; then
    echo "Script compiled successfully: $SCRIPT_NAME.so"
else
    echo "Failed to compile script: $SCRIPT_FILE"
    exit 1
fi

echo "Build complete!"
