# Right now:
Todo:
- Figure out stuttering bug with multiple clients
    - Allow network logging with multiple clients
- Generate map at runtime on server, along with single base location


# Considering:
- Allow client to run without server
    - Create a Transport layer on the client side, one with ENet and one that's a mock.
    - Templatify Network and main method for client
    - Create a mock server layer which consumes packets from the transport layer and passes back mocked responses
- Login screen to synchronize players
    - How to keep development modular?
- Multiple bases?! Keyblocks?!
- Think more about jumping
    - Remove jumping?
    - Add jumping into movement input bit mask?
    - Jumping animations?
- Combat???
