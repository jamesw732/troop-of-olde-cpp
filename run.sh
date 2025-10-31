#!/usr/bin/env bash

CLIENTONLY=0
SERVERONLY=0
DEBUG=0
TARGET="all"
for arg in "$@"; do
    if [ "$arg" == "client" ]; then
        CLIENTONLY=1
    elif [ "$arg" == "server" ]; then
        SERVERONLY=1
    elif [ "$arg" == "debug" ]; then
        TARGET="debug"
        DEBUG=1
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
        gnome-terminal -- gdbgui ./server_debug
    fi
    if [ $SERVERONLY == 0 ]; then
        gnome-terminal -- gdbgui ./client_debug
    fi
elif [ "$TARGET" == "tests" ]; then
    if [ $CLIENTONLY == 1 ]; then
        echo "Running client tests"
        ctest -R client* --output-on-failure
    elif [ $SERVERONLY == 1 ]; then
        echo "Running server tests"
        ctest -R server* --output-on-failure
    else
        echo "Running all tests"
        ctest --output-on-failure
    fi
elif [[ ${TARGET#*.} == test* ]]; then
    echo "Running test file ${TARGET}"
    if [ $DEBUG == 1 ]; then
        gnome-terminal -- gdbgui ./$TARGET
    else
        ctest -R $TARGET --output-on-failure
    fi
else
    echo "Running main build"
    if [ $CLIENTONLY == 0 ]; then
        gnome-terminal -- ./server
    fi
    if [ $SERVERONLY == 0 ]; then
        gnome-terminal -- ./client
    fi
fi

