#include "raylib.h"
#include "flecs.h"
#include <iostream>

const float MOVE_UPDATE_RATE = 1.0 / 20.0;

struct Transformation {
    Vector3 pos;
    Vector3 rot;
    Vector3 scale;
};

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    SetTargetFPS(60);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    flecs::world world;

    Transformation transformComp = {
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 1.0f}
    };

    auto e = world.entity("character");
    e.add<Transformation>();
    e.set<Transformation>(transformComp);
    auto move_sys = world.system<Transformation>()
        .interval(MOVE_UPDATE_RATE)
        .each([](flecs::iter& it, size_t, Transformation& t) {
                if (IsKeyDown(KEY_W)) {
                    t.pos.z -= 0.25;
                }
                if (IsKeyDown(KEY_S)) {
                    t.pos.z += 0.25;
                }
                if (IsKeyDown(KEY_D)) {
                    t.pos.x += 0.25;
                }
                if (IsKeyDown(KEY_A)) {
                    t.pos.x -= 0.25;
                }
            }
        );
    auto render_sys = world.system<Transformation>()
        .each([&camera](flecs::iter& it, size_t, Transformation& t) {
                BeginMode3D(camera);
                    DrawCube(t.pos, 2.0f, 2.0f, 2.0f, RED);
                    DrawCubeWires(t.pos, 2.0f, 2.0f, 2.0f, MAROON);
                EndMode3D();
            }
        );

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        world.progress(dt);
        // Do rendering
        BeginDrawing();
            // Draw the UI
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                DrawGrid(10, 1.0f);
            EndMode3D();
            DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
            DrawFPS(10, 10);
            // Draw each entity in the scene
            render_sys.run();
        EndDrawing();
    }
    CloseWindow();

    return 0;
}