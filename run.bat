@echo off
SET SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%\build"
cmake -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++.exe -DCMAKE_BUILD_TYPE=Debug ..
mingw32-make

start cmd /k server.exe
start cmd /k client.exe
