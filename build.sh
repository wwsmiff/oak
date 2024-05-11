#!/bin/bash

set -xe

CC="g++"
INCLUDE_DIR="./include"
SRC_DIR="./src/*.cpp"
BUILD_DIR="./build"
CFLAGS="-Wall -std=c++20 -O2 -pedantic"
LFLAGS=""
EXE="main"

$CC -I$INCLUDE_DIR $CFLAGS $SRC_DIR -o "$BUILD_DIR/$EXE" $LFLAGS
