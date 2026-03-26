# Dependencies
Any C++ compiler and CMake.

Code dependencies are listed in `deps` as Git submodules. To pull along with the submodules,
run `git clone --recurse-submodules <repository-url>`. Or if you already pulled and want to grab the
submodules, run `git submodule update --init --recursive` from `deps`.


# Compiling
This project uses CMake for compilation. Typical usage should follow standard CMake patterns, but
potential deviations are covered here.

## WSL
You can compile this project and run it from WSL if you use the MinGW cross compiler. You will
need to compile for Windows, since this is a graphical application. 

Assuming you have MinGW installed, from the root of the repository run:
`cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake`
This will generate build scripts for compiling with MinGW, and you should be able to proceed normally.

## Main Executables
```
mkdir build    (if not on WSL)
cd build
cmake ..
make
```
Note: If this is your first time running `cmake`, you may have to do some configuration beforehand.

Building will create two executables: `client`, and `server`.

## Tests
To build the tests, instead run `make tests`. This will create `test_client` and `test_server` executables
which run all the unit tests for the client and server. Run `ctest` to execute all tests.

## Debug
To compile with debug flags, run `make debug`. This will create `client_debug` and `server_debug` executables.

## Dependencies
To compile dependencies without compiling any `client` or `server` executables, run `make deps`.


# Running
As a brief overview, building the project will generate two executables, a `server` and a `client`.
Currently, the server is hardcoded to host a localhost server, and the client is
hardcoded to connect to it. So to launch the client and server, simply run (in separate terminals):
The naive way to launch the game is to run the server executable:
`./server` (`./server.exe` on Windows)
Then, in a separate terminal, run the client executable:
`./client` (`./client.exe` on Windows)

I have made some attempts to automate this process along with compilation.
## Tmux + Vim (WSL/Linux)
Most of my development is in a Tmux + Vim environment, to get this working you will want to copy
the `dev.tmux.example` file, into say `dev.tmux`, and run `./dev.tmux`. This will open persistent
terminals which can run a server and a client.

Add this to your `vimrc`, and you will be able to build and run with `<leader>r`, or build with `<leader>b`:
```
" Build troop-of-olde-cpp
let g:tmux_session = "dev"
let g:tmux_build_pane  = "0.0"
let g:tmux_server_pane = "0.1"
let g:tmux_client_pane = "0.2"
let g:build_command    = "cmake --build build"
set makeprg=cmake\ --build\ build

function! FindProjectRoot()
  let l:root = findfile('CMakeLists.txt', expand('%:p:h') . ';')
  if empty(l:root)
    echohl ErrorMsg | echo "CMakeLists.txt not found" | echohl None
    return ''
  endif
  return fnamemodify(l:root, ':h')
endfunction

function! Rebuild()
  let l:root = FindProjectRoot()
  let l:cwd = getcwd()
  if empty(l:root)
    return
  endif
 " Run build from project root
  execute 'lcd ' . fnameescape(l:root)
  make

  if v:shell_error != 0
    echohl ErrorMsg | echo "Build failed" | echohl None
  endif
  execute 'lcd ' . fnameescape(l:cwd)
endfunction

function! RebuildAndRestart()
  call Rebuild()

  " Restart server
  silent !tmux send-keys -t dev:0.1 C-c
  silent !tmux send-keys -t dev:0.1 -l "./server.exe"
  silent !tmux send-keys -t dev:0.1 C-m

  " Restart client
  silent !tmux send-keys -t dev:0.2 C-c
  silent !tmux send-keys -t dev:0.2 -l "./client.exe"
  silent !tmux send-keys -t dev:0.2 C-m
endfunction

nnoremap <leader>b :call Rebuild()<CR>
nnoremap <leader>r :call RebuildAndRestart()<CR>
```

## Linux
On Linux, you can use the (now deprecated) `run.sh` script, which compiles the program and automatically
launches two terminals (specifically gnome-terminal, but this is plug and play), one that starts a server
and one that starts a client.
- `client` builds and runs only client executables, can be used with other options
- `server` builds and runs only server executables, can be used with other options
- `debug` builds the debug executables
- `tests` builds the unit tests. Can instead specify specific test files to run, using the command
`./run.sh [client | server].[test file name]`, for example
`./run.sh client.test_movement`.

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

