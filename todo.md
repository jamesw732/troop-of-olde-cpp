# Right now:


# Coming soon:

## Stop handling movement inputs every frame, instead do it every 1/20s
There are three procedures that need to happen:
1. Collect player movement inputs,
2. Predict client-side movement,
3. Send inputs to server

2 and 3 should probably be decoupled. This means that 1 should be decoupled from 2 and 3.
1 will also eventually involve somewhat advanced input buffering for redundant input networking,
so it should definitely be handled separately.

OK, I think this should be implemented as 3 independent singleton Systems, all operating on the same
fixed timestep, and in order. InputHandler is a helper class for handling inputs, its methods will be
pure functions.

0. Rename KeyboardMovement to MovementInput, this will eventually encode keyboard movement, keyboard
rotation, and mouse rotation (the latter two aren't even implemented client-side). These will be
stored in a MovementInputBuffer for redundant input networking, but this is unnecessary right now.
1. Movement inputs system: mutates the singleton MovementInput with the current keyboard (and eventually
mouse) input state.
2. Movement system: reads from the singleton MovementInput and player character's position
(and eventually rotation, and server offsets) 
3. Movement networking system: reads from the singleton MovementInput to send inputs to server






## Make server respond with position updates, make client reconcile those updates

## Linearly interpolate movement every frame, in between movement updates

## Allow for more packet types than just movement

## Make server responsible for creating client's character / allow for multiple clients to exist in the game

## Redundant movement input networking
