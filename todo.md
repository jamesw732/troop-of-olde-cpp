# Right now:

## Collision physics
- Add ground terrain (done)
- Split up movement input processing from position updates, update a Velocity variable (done)
- Refactor Rotation components to use a Vector3 val rather than a float for just the y rotation. (done)
- Write a generic GetRayIntersection function which queries all terrain elements and calls one of the
various collision functions depending on the model. (done)
- Test the collision function by shooting a ray downwards, and testing whether it hits floor. (done)
- Gravity (done)
- Jumping (done)
- Terrain collision

# Coming soon:

## Load character model

## Animate character movement
