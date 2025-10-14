# Dependencies
Header-only libraries (ie ENet, raylib-cpp) are presumed to exist in a default global include
directory (on linux, `/usr/local/include`).

Static libraries (ie raylib, flecs) and their dependencies are presumed to exist in a default global
location (on linux, `/usr/local/lib`). Each library listed may have platform-dependent installation processes.

- raylib: Download (or compile from source) and install from one of the links:
[raylib website](https://www.raylib.com/), [raylib repository](https://github.com/raysan5/raylib)
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
a location your C++ compiler can see.
- raylib-cpp: Download/clone from [raylib-cpp repository](https://github.com/RobLoach/raylib-cpp)
and make the `include` directory accessible to the compiler (copy contents to `/usr/local/include`).
- ENet: Download/clone from [the ENet repository](https://github.com/zpl-c/enet) and make
`include/enet.h` accessible to the compiler (copy to `/usr/local/include`).
- Bitsery: Download/clone from [the Bitsery repository](https://github.com/fraillt/bitsery/tree/master)
and make `include/bitsery` accessible to the compiler (copy to `/usr/local/include`).


# Compiling and Running
This repository uses `cmake` for building. To build, simply run the following in the root:
```
mkdir build
cd build
cmake ..
make
```
Building will create two executables: `client`, and `server`.
Currently, the server is hardcoded to host a localhost server, and the client is
hardcoded to connect to it. So to launch the client and server, simply run (in separate terminals):
```
./server
```

```
./client
```

To build the tests, run `make tests`. This will create `test_client` and `test_server` executables
which run all the unit tests for the client and server.

To compile with debug flags, run `make debug`.

# Running

## Testing with simulated networking conditions
On Linux, the following command can be used to simulate network conditions (100ms latency, 20ms jitter,
1% packet loss):
```
sudo tc qdisc add dev lo root netem delay 100ms 20ms loss 1%
```
To reset:
```
sudo tc qdisc del dev lo root
```
