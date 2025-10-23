#!/usr/bin/env bash

CLIENTONLY=0
SERVERONLY=0
TARGET="all"
for arg in "$@"; do
    if [ "$arg" == "client" ]; then
        CLIENTONLY=1
    elif [ "$arg" == "server" ]; then
        SERVERONLY=1
    elif [ "$arg" == "debug" ]; then
        TARGET="debug"
    elif [ "$arg" == "tests" ]; then
        TARGET="tests"
    elif [[ ${arg#*.} == test* ]]; then
        TARGET=$arg
    fi
done

ROOT_DIR=$(dirname "$0")
BUILD_DIR="$ROOT_DIR/build"

cd $BUILD_DIR
make $TARGET

if [ "$TARGET" == "debug" ]; then
    echo "Running debug build"
    if [ $CLIENTONLY == 0 ]; then
        gnome-terminal -- gdb ./server_debug
    fi
    if [ $SERVERONLY == 0 ]; then
        gnome-terminal -- gdb ./client_debug
    fi
elif [ "$TARGET" == "tests" ]; then
    echo "Running tests"
    if [ $CLIENTONLY == 1 ]; then
        ctest -R client*
    elif [ $SERVERONLY == 1 ]; then
        ctest -R server*
    else
        ctest
    fi
elif [[ ${TARGET#*.} == test* ]]; then
    ctest -R $TARGET
else
    echo "Running main build"
    if [ $CLIENTONLY == 0 ]; then
        gnome-terminal -- ./server
    fi
    if [ $SERVERONLY == 0 ]; then
        gnome-terminal -- ./client
    fi
fi

