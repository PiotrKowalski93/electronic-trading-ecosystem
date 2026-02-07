#!/bin/bash

TESTS_DIR=build-debug/tests

cd "$TESTS_DIR"

echo "===> Running tests"
ctest --verbose