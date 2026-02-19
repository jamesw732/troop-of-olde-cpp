# Right now:
- Refactor code to be more cache and ECS friendly
    - Make client's input buffer be a ring array
    - Make server-side input buffers persistent components
    - Make server and client consistent about component names
        - Sim* means it's a server-authoritative state.
        - Pred* means it's a predicted state, not authoritative due to either missing inputs
            from client or missing movement updates from server
        - PrevPred* is the previous predicted state, used by client for lerping
        - Render* is the state drawn on screen, interpolated between movement ticks

- Can jump through the wall with >90 degree incline. Consider a fix for this, or just never
generate meshes with this property.

- Add basic "3d quad" and "cube" models to the model map, and remove ModelType component/handling

# Coming soon:

## Load character model

## Animate character movement

## Combat???
