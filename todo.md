# Right now:


# Coming soon:

## Movement networking
Algorithm:
- Every 0.05s, client will poll for keyboard inputs for movement, and immediately perform simulation
locally to compute move location. Increments the local "movement tick", and also stores the following
in buffers:
    movement tick, movement input
    movement tick, predicted position/rotation.
Then sends the entire "movement tick to movement input" buffer to the server.
- Server receives buffer, and simulates movement for all movement inputs received with movement tick higher
than the previous highest movement tick received for this client. Then updates the current movement tick.
Server sends back to the client the current stored movement tick, and the currently stored position.
- Client replays simulation from authoritative server update, and flushes both buffers up to the
acknowledged movement tick. This makes it so that the client doesn't send all movement inputs, only
the ones the server doesn't know about yet.


### Make server respond with position updates
- Server will receive movement inputs and current client tick. Will update local position and rotation.
- Server will send back position and rotation for the most recent client tick received.
Use Bitsery for serialization, this will be 

### Make client track movement ticks,
- Additionally, make two maps:
    - tick number to movement input (not used yet)
    - tick number to predicted position/rotation (used for reconciling)

### Make client reconcile server updates
- Client receives position and rotation along with a tick number, compares received position and rotation
with the position and rotation stored for that tick. Flushes the buffer for each tick up to that tick.
Stores the offset in a component
- In the existing movement system, also examines the offsets when updating the state.

### Redundant movement input networking
- Similar to how we store position and rotation for each movement tick, we should also store the movement
inputs for that tick
- Rather than send the current movement inputs for this tick, send everything we have stored
- Server will read potentially several movement inputs, updating state from each one
- When client receives update and tick acknowledgement, also flush the bottom of the input buffer.

## Linearly interpolate movement every frame, in between movement updates

## Allow for more packet types than just movement

## Make server responsible for creating client's character / allow for multiple clients to exist in the game
