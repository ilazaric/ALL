#include "raylib.h"

int main()
{
  const int screenW = 800, screenH = 450;
  InitWindow(screenW, screenH, "raylib‑wasm demo");
  SetTargetFPS(60);

  // Texture2D tex = LoadTexture("assets/player.png");

  Image img = LoadImage("assets/player.png");
  ImageResize(&img, 128, 128);
  Texture2D tex = LoadTextureFromImage(img);
  UnloadImage(img);

  Vector2 pos = { screenW/2.f - tex.width/2.f, screenH/2.f - tex.height/2.f };

  while (!WindowShouldClose())
    {
      const auto dt = GetFrameTime();
      const auto dx = dt * 300;
      const auto dy = dt * 300;
      
      if (IsKeyDown(KEY_W)) pos.y -= dy;
      if (IsKeyDown(KEY_S)) pos.y += dy;
      if (IsKeyDown(KEY_A)) pos.x -= dx;
      if (IsKeyDown(KEY_D)) pos.x += dx;

      BeginDrawing();
      ClearBackground(RAYWHITE);
      DrawTexture(tex, pos.x, pos.y, WHITE);
      EndDrawing();
    }

  UnloadTexture(tex);
  CloseWindow();
  return 0;
}
