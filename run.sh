#!/usr/bin/env bash

COMPILER=/usr/bin/clang++-20
TERM_COMMAND=gnome-terminal

CLIENTONLY=0
SERVERONLY=0
DEBUG_FLAGS="-DCMAKE_BUILD_TYPE=Debug"
TARGET="all"
RUN_COMMAND=""
for arg in "$@"; do
    if [ "$arg" == "client" ]; then
        CLIENTONLY=1
    elif [ "$arg" == "server" ]; then
        SERVERONLY=1
    elif [ "$arg" == "debug" ]; then
        RUN_COMMAND="gdbgui"
    elif [ "$arg" == "debug-console" ]; then
        RUN_COMMAND="gdb"
    elif [ "$arg" == "tests" ]; then
        TARGET="tests"
    elif [[ ${arg#*.} == test* ]]; then
        TARGET=$arg
    elif [ "$arg" == "release" ]; then
        DEBUG_FLAGS=""
    fi
done

ROOT_DIR=$(dirname "$0")
BUILD_DIR="$ROOT_DIR/build"

cd $BUILD_DIR

echo $DEBUG_FLAGS
CONFIG_FLAGS=(-DCMAKE_CXX_COMPILER=$COMPILER $DEBUG_FLAGS)
cmake "${CONFIG_FLAGS[@]}" ..

make $TARGET

if [ "$TARGET" == "tests" ]; then
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
else
    if [ $CLIENTONLY == 0 ]; then
        $TERM_COMMAND -- $RUN_COMMAND ./server
    fi
    if [ $SERVERONLY == 0 ]; then
        $TERM_COMMAND -- $RUN_COMMAND ./client
    fi
fi

# if [ "$TARGET" == "debug" ]; then
#     echo "Running debug build"
#     if [ $CLIENTONLY == 0 ]; then
#         gnome-terminal -- gdbgui ./server_debug
#     fi
#     if [ $SERVERONLY == 0 ]; then
#         gnome-terminal -- gdbgui ./client_debug
#     fi
# elif [ "$TARGET" == "tests" ]; then
#     if [ $CLIENTONLY == 1 ]; then
#         echo "Running client tests"
#         ctest -R client* --output-on-failure
#     elif [ $SERVERONLY == 1 ]; then
#         echo "Running server tests"
#         ctest -R server* --output-on-failure
#     else
#         echo "Running all tests"
#         ctest --output-on-failure
#     fi
# elif [[ ${TARGET#*.} == test* ]]; then
#     echo "Running test file ${TARGET}"
#     if [ $DEBUG == 1 ]; then
#         gnome-terminal -- gdbgui ./$TARGET
#     else
#         ctest -R $TARGET --output-on-failure
#     fi
# else
#     echo "Running main build"
#     if [ $CLIENTONLY == 0 ]; then
#         gnome-terminal -- ./server
#     fi
#     if [ $SERVERONLY == 0 ]; then
#         gnome-terminal -- ./client
#     fi
# fi

