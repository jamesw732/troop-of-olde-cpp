# Dependencies
All code dependencies are included in the `deps` directory and will be compiled automatically.

# Compiling and Running
## Main Executables
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

## Tests
To build the tests, instead run `make tests`. This will create `test_client` and `test_server` executables
which run all the unit tests for the client and server. Run `ctest` to execute all tests.

## Debug
To compile with debug flags, run `make debug`.

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
