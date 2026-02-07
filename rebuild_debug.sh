#!/bin/bash
set -e  # przerwij skrypt przy pierwszym błędzie

BUILD_DIR=build-debug

echo "===> Cleaning build dir"
rm -rf "$BUILD_DIR"

echo "===> Creating build dir"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

echo "===> Configuring"
cmake -DCMAKE_BUILD_TYPE=Debug ..

echo "===> Building"
cmake --build . -j$(nproc)

echo "===> Running tests"
ctest --verbose