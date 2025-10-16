# Dependencies
Any C++ compiler and CMake.

Code dependencies are listed in `deps` as Git submodules. To pull along with the submodules,
run `git clone --recurse-submodules <repository-url>`. Or if you already pulled and want to grab the
submodules, run `git submodule update --init --recursive` from `deps`.

However, it is not required to download the submodules. If you don't, CMake will download them for you
in the build directory.

# Compiling and Running
## Main Executables
This repository uses `cmake` for building. To build, simply run the following in the root:
```
mkdir build
cd build
cmake ..
make
```
Note: If this is your first time running `cmake`, you may have to do some configuration beforehand.

Building will create two executables: `client`, and `server`.
Currently, the server is hardcoded to host a localhost server, and the client is
hardcoded to connect to it. So to launch the client and server, simply run (in separate terminals):
```
./server
```

```
./client
```

## Tests
To build the tests, instead run `make tests`. This will create `test_client` and `test_server` executables
which run all the unit tests for the client and server. Run `ctest` to execute all tests.

## Debug
To compile with debug flags, run `make debug`. This will create `client_debug` and `server_debug` executables.

## Dependencies
To compile dependencies without compiling any `client` or `server` executables, run `make deps`.

## Simulating network conditions
On Linux, the following command can be used to introduce artificial latency, jitter, and
packet loss (100ms latency, 20ms jitter, 1% packet loss):
```
sudo tc qdisc add dev lo root netem delay 100ms 20ms loss 1%
```
To reset:
```
sudo tc qdisc del dev lo root
```
