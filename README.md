# Dependencies
Any C++ compiler and CMake.

Code dependencies are listed in `deps` as Git submodules. To pull along with the submodules,
run `git clone --recurse-submodules <repository-url>`. Or if you already pulled and want to grab the
submodules, run `git submodule update --init --recursive` from `deps`.


# Compiling
This project uses CMake for compilation. Typical usage should follow standard CMake patterns, but
potential deviations are covered here.

See [WSL instructions](#wsl) for compiling on WSL.
```
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Building will create two executables: `client`, and `server`, which you can run with `./client` and `./server`.
On windows, they will have the `.exe` extension, so you will need to run them with `./client.exe` and `./server.exe`.

<!-- ## Tests -->
<!-- Unit tests are not currently maintained. They will not even compile right now. -->

<!-- To build the tests, instead run `make tests`. This will create `test_client` and `test_server` executables -->
<!-- which run all the unit tests for the client and server. Run `ctest` to execute all tests. -->

## WSL
You can compile this project and run it from WSL if you use the MinGW cross compiler. You will
need to compile for Windows, since this is a graphical application. 

You can configure your build scripts to use MinGW by passing in the toolchain:
```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```
Or for MinGW Clang:
```
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-clang-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

I have had some trouble with clangd while using MinGW, it seems to get very confused by the conflicting
environment. This .clangd works for me:
```
CompileFlags:
  Add: [
    -std=c++20,
    -I/abs/path/to/repo/root/deps/bitsery/include,
    -I/abs/path/to/repo/root/deps/raylib/src,
    -I/abs/path/to/repo/root/deps/enet/include,
    -I/abs/path/to/repo/root/deps/flecs/include
]
  Remove: [-include, build/CMakeFile/pch.dir/pch.hpp/gch]

QueryDriver:
  - /usr/bin/x86_64-w64-mingw32-*

Diagnostics:
  Suppress:
    - clang-diagnostic-unused-include
```

To compile this README and view it in a browser all from WSL, run:
```
pandoc README.md -o readme.html
powershell.exe -c "start msedge '$(wslpath -w readme.html)'"
```

# Tmux + Vim (WSL/Linux)
Most of my development is in a Tmux + Vim environment in WSL, to get this working you will want to copy
the `dev.tmux.example` file, into say `dev.tmux`, and run `./dev.tmux`. This will open persistent
terminals which can run a server and a client.

This project includes a `.project.vim` file, which contains vim config to allow you to easily build and run
the project within vim. Once you source `.project.vim`, you will be able to build and run with `<leader>r`, or
just build with `<leader>b`.

To automatically source this vim config, I suggest adding this to your global .vimrc:
```
" Load project-specific vimrc
let s:project_vimrc = findfile('.project.vim', '.;')
if !empty(s:project_vimrc)
    execute 'source' fnameescape(s:project_vimrc)
endif
```

# Simulating network conditions
On Linux, the following command can be used to introduce artificial latency, jitter, and
packet loss (100ms latency, 20ms jitter, 1% packet loss):
```
sudo tc qdisc add dev lo root netem delay 100ms 20ms loss 1%
```
To reset:
```
sudo tc qdisc del dev lo root
```

# Binary Logging
The server and client generate binary log files for all networking traffic, separate logs for
outgoing and incoming. These live in the build directory along with the executables. After running
the server and client for some time, you can run the `logger` executable, for example:
`./logger.exe client-in.bin > client-in.log`

Note, client logging is currently broken with multiple clients because all clients write to the same file.

