# Right now:

## Collision physics
- Ground (done)
- Gravity
    - Split up movement input processing from position updates, update a Velocity component
    - Spawn character at nonzero height
    - Add gravity to velocity when not grounded
    - When processing velocity, query entities with a Terrain component. Check ray collision with
    those entities, return collision with closest entity. If hit, set grounded.
- Jumping
- Terrain collision

# Coming soon:

## Load character model

## Animate character movement
