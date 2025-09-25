# Right now:

## Movement networking
Algorithm:
Client stores its current movement tick (increments at 20hz), the currently acknowledged movement tick
by the server, and an input buffer containing every movement input between the acknowledged tick
and the current movement tick. It should also store its position (eventually, rotation) at each tick,
in a hashmap? (done)

Every movement tick, the client sends the whole input buffer and current movement tick to the server.
The server computes the new position by applying each movement input instantly. It sends the updated
position back to the client along with the movement tick. (done)

Upon receiving the new position and movement tick, the client looks up the position it predicted at
that movement tick. It re-applies all movements on top of that position instantly to compute the next
predicted position. The client also flushes the buffer up to the acknowledged tick.


Just need to make client handle the received tick and position correctly.




# Coming soon:




## Linearly interpolate movement every frame, in between movement updates

## Allow for more packet types than just movement

## Make server responsible for creating client's character / allow for multiple clients to exist in the game
