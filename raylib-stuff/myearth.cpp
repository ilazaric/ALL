#include "raylib.h"
#include "raymath.h"

#include <sstream>

int main(void)
{
  const int W = 1960, H = 1080;
  // const int W = 1280, H = 720;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(W, H, "raylib • Earth");
  // DisableCursor();
  SetTargetFPS(60);

  Image img = LoadImage("assets/8k_earth_daymap.jpg");
  ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
  ImageRotate(&img, 90); // don't ask
  ImageFlipHorizontal(&img);
  Texture2D tex = LoadTextureFromImage(img);
  UnloadImage(img);

  // GenTextureMipmaps(&tex);                              // <‑‑ add this
  // SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);        // better mip‑maps
  // SetTextureWrap  (tex, TEXTURE_WRAP_REPEAT);
  SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);        // better mip‑maps
 
  // 1) Generate a UV‑mapped sphere mesh and wrap an Earth texture around it
  Mesh sphere   = GenMeshSphere(1.0f, 128, 256);      // radius 1
  Model earth   = LoadModelFromMesh(sphere);
  earth.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = tex;

  // 2) Basic camera that orbits the Y‑axis with right‑drag
  Camera3D cam = { 0 };
  cam.position = (Vector3){ 0.0f, 3.0f, 0.0f };      // start 3 units back
  cam.target   = (Vector3){ 0.0f, 0.0f, 0.0f };
  cam.up         = { 0.0f, 0.0f, 1.0f };   // Y‑axis is “up”
  cam.fovy     = 45.0f;
  cam.projection = CAMERA_PERSPECTIVE;

  float yaw = 0.0f, pitch = 0.0f;
  const float sens = 0.003f;
  float dist = 3.0f;

  while (!WindowShouldClose())
    {
      // --- camera control ------------------------------------------------
      if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
        {
	  const auto dist_coef = (dist - 1.0f) * 0.5f;
	  Vector2 delta = GetMouseDelta();
	  yaw   += delta.x * sens * dist_coef;
	  pitch += delta.y * sens * dist_coef;
	  pitch = Clamp(pitch, -PI/2.0f + 0.1f, PI/2.0f - 0.1f);
        }

      dist -= GetMouseWheelMove() * 0.1f;

      // Spherical to Cartesian
      cam.position.x = dist * cosf(pitch) * sinf(yaw);
      cam.position.z = dist * sinf(pitch);
      cam.position.y = dist * cosf(pitch) * cosf(yaw);
      UpdateCamera(&cam, CAMERA_CUSTOM);

      Vector2 delta = GetMouseDelta();
      std::stringstream bla;
      bla << delta.x << " " << delta.y << " " << GetMouseWheelMove();

      // --- draw ----------------------------------------------------------
      BeginDrawing();
      ClearBackground(BLACK);
      BeginMode3D(cam);
      DrawModel(earth, Vector3Zero(), 1.0f, WHITE);
      // DrawGrid(10, 1.0f);
      EndMode3D();
      DrawFPS(10, 10);
      DrawText("RMB drag to rotate  •  Scroll to zoom (optional)", 10, 30, 20, GRAY);
      DrawText(bla.str().c_str(), 10, 50, 20, GRAY);
      EndDrawing();
    }

  UnloadTexture(tex);
  UnloadModel(earth);
  CloseWindow();
  return 0;
}
