# Right now:

## Move to multiple files.
Will need to make some globals in order for system functions to be
definable outside of main. Probably just peer.


# Coming soon:

## Stop handling movement inputs every frame, instead do it every 1/20s
Should make a new system which only touches the player character.
Will need to create a new "PC" component which stores no data, just a flag.

## Make server respond with position updates, make client reconcile those updates

## Linearly interpolate movement every frame, in between movement updates

## Allow for more packet types than just movement

## Make server responsible for creating client's character / allow for multiple clients to exist in the game
