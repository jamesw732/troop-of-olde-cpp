# Dependencies
- raylib: Download (or compile from source) and install from one of the links:
[raylib website](https://www.raylib.com/), [raylib repository](https://github.com/raysan5/raylib)
- raylib-cpp: Download (or compile from source) and install from
[raylib-cpp repository](https://github.com/RobLoach/raylib-cpp)
- flecs: This repository depends on a statically compiled `flecs` library. Installing flecs by
following the official install instructions is not straightforward, but it is easy to install
with `cmake`.  Clone flecs from [the flecs repository](https://github.com/SanderMertens/flecs)
and compile it using `cmake`:
```
    mkdir build
    cd build
    cmake ..
    sudo make install
```
Alternatively, it should be possible to run `make` and then manually copy `libflecs_static.a` into
a location your C++ compiler can see (on linux

- ENet: Follow the installation instructions from
[the ENet repository](https://github.com/zpl-c/enet). You just need to download `enet.h` and make
it accessible to the source code.


Each library listed may have platform-dependent installation processes.

Header-only libraries (ie ENet, raylib-cpp) are presumed to exist in a default global include
directory (on linux, `/usr/local/include`).

Static libraries (ie raylib, flecs) and their dependencies are presumed to exist in a default global
location (on linux, `/usr/local/lib`).


# Compiling
This repository uses `cmake` for building. To build, simply run the following in the root:
```
mkdir build
cd build
cmake ..
make
```

# Running
Building will create two executables: `client`, and `server`.
Currently, the server is hardcoded to host a localhost server, and the client is
hardcoded to connect to it. So to launch the client and server, simply run (in separate terminals):
```
./server
```

```
./client
```
