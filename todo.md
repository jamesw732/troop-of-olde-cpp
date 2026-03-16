# Right now:

## Animate character movement
- Animation loading
    - New map of string (model name) to map of string (animation name) to ModelAnimation
    - Wrap LoadModelAnimations to insert a model's animations into animation map
- Animation state machine
    - Decide how to increment AnimationTimer - standalone simple system?
- Compute poses
    - Store previous pose when switching to new one
    - Interpolate between previous pose and current pose for blending with growing alpha factor
- Render models with pose applied


# Coming soon:


## Combat???
