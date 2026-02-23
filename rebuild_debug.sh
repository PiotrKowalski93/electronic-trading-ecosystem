#!/bin/bash
#TODO: add comments about params
set -e

BUILD_DIR=build-debug

echo "===> Configuring (Debug)"
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DCMAKE_CXX_FLAGS_DEBUG="-O0 -g3 -fno-omit-frame-pointer" \

echo "===> Building"
cmake --build build-debug -j$(nproc)