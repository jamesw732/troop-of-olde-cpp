#include "raylib.h"
#include <iostream>
#include <vector>


struct TransformComponent {
    Vector3 pos;
    Vector3 rot;
    Vector3 scale;
};

void RenderSystem(std::vector<TransformComponent> iter, Camera3D camera) {
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
            // Draw each entity in the scene
            for (TransformComponent comp: iter) {
                DrawCube(comp.pos, 2.0f, 2.0f, 2.0f, RED);
                DrawCubeWires(comp.pos, 2.0f, 2.0f, 2.0f, MAROON);
            }
            DrawGrid(10, 1.0f);
        EndMode3D();
        DrawText("Welcome to the third dimension!", 10, 40, 20, DARKGRAY);
        DrawFPS(10, 10);
    EndDrawing();
}

void MovementSystem(std::vector<TransformComponent> &iter) {
    for (TransformComponent &comp: iter) {
        if (IsKeyDown(KEY_W)) {
            comp.pos.z -= 0.1;
        }
        if (IsKeyDown(KEY_S)) {
            comp.pos.z += 0.1;
        }
        if (IsKeyDown(KEY_D)) {
            comp.pos.x += 0.1;
        }
        if (IsKeyDown(KEY_A)) {
            comp.pos.x -= 0.1;
        }
    }
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [core] example - 3d camera mode");

    // Define the camera to look into our 3d world
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    TransformComponent transformComp = {
         {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 1.0f}
    };
    std::vector<TransformComponent> transformComps;
    transformComps.push_back(transformComp);

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        MovementSystem(transformComps);
        RenderSystem(transformComps, camera);
    }
    CloseWindow();

    return 0;
}