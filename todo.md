# Right now:

## Windows build
Need to solve the enet/raylib conflict.
No matter what, this will involve isolating the network interface from any raylib.h includes,
and also compiling the network source code as a separate static library.
Basically, any file should interface to client/network.hpp or server/network.hpp if they want
to do any sort of networking, but neither header should include enet.h.
Instead, client/network.cpp and server/network.cpp should include enet.h
Will need to do some forward declare magic to avoid exposing ENet types in the headers
Beyond this, the biggest issue will be serializing raylib types, I have two ideas for how to do this:
### Make network layer as shallow as possible (better)
- If network doesn't handle serialization/deserialization, don't need to worry about raylib types
- Would require some refactoring, namely moving game object creation and deserialization out of network.hpp.
- This seems like something I might want to do anyways? Keep network more single-responsibility.
### Make own wrapper for raylib types
- The problem is that vector3.hpp and every other raylib-cpp files include raylib.h, so the parts of raylib-cpp
that are relevant for networking would need to be re-wrapped given the current structure of the code.
- Right now, would just need to wrap Vector3
- This would be the quick and easy way out, but introduces some awkwardness.
- Not quite future proof, could have to wrap more types in the future

### Steps
- Make all enet calls go through Network (done)
- Separate serialize/deserialize logic from Network implementation
- Separate Network implementation from interface


# Coming soon:

## Unit tests


## Content
### Implement character rotation
