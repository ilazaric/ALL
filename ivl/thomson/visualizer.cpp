#include "point"
#include <raylib/raylib.h>
#include <raylib/raymath.h>

// IVL add_compiler_flags("-fno-strict-aliasing")
// IVL add_compiler_flags_tail("-L/home/ilazaric/repos/ALL/submodules/objdir/raylib/raylib/ -lraylib")
// IVL add_compiler_flags_tail("-lm  -lpthread -lOpenGL  -lGLX  -lGLU  -lm  -lrt  -lm  -ldl")

Vector3 point2vec(point p) { return Vector3{(float)p.x, (float)p.y, (float)p.z}; }

void normalize(std::span<point> points) {
  for (auto& p : points) p = p / norm(p);
}

void draw_circle(Vector3 center, float radius, Vector3 normal, Color color) {
  Vector3 a = Vector3Perpendicular(normal);
  Vector3 b = Vector3CrossProduct(normal, a);
  Vector3 last = Vector3Add(center, Vector3Scale(a, radius));
  for (int i = 10; i <= 360; i += 10) {
    Vector3 next = Vector3Add(
      center, Vector3Add(Vector3Scale(a, cos(DEG2RAD * i) * radius), Vector3Scale(b, sin(DEG2RAD * i) * radius))
    );
    DrawLine3D(last, next, color);
    last = next;
  }
}

void draw_arc(Vector3 a, Vector3 b, Color color) {
  Vector3 last = a;
  for (int i = 1; i <= 10; ++i) {
    Vector3 next = Vector3Normalize(Vector3Lerp(a, b, (float)i / 10.0f));
    DrawLine3D(last, next, color);
    last = next;
  }
}

int ivl_main(std::size_t point_count) {
  const int screenWidth = 1000;
  const int screenHeight = 1000;
  const Vector3 sphere_center{0.0, 0.0, 0.0};
  const double sphere_radius = 1.0;
  const Color sphere_color = ColorAlpha(RED, 0.5);
  const Color point_color = GREEN;
  const float point_radius = 0.01f;
  std::vector<point> points(point_count);
  for (auto& p : points) p = random_point();
  normalize(points);
  SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(screenWidth, screenHeight, "visualizer");

  Camera camera = {0};
  camera.position = (Vector3){0.0f, 2.0f, 2.0f};
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};
  camera.up = (Vector3){0.0f, 1.0f, 0.0f};
  camera.fovy = 45.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  SetTargetFPS(60);
  float rotation_speed = 1.0f;
  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    if (IsKeyPressed(KEY_KP_ADD)) rotation_speed *= 2;
    if (IsKeyPressed(KEY_KP_SUBTRACT)) rotation_speed /= 2;
    {
      float dx = 0;
      float dy = 0;
      if (IsKeyDown(KEY_W)) --dy;
      if (IsKeyDown(KEY_S)) ++dy;
      if (IsKeyDown(KEY_A)) ++dx;
      if (IsKeyDown(KEY_D)) --dx;
      dx *= dt * rotation_speed;
      dy *= dt * rotation_speed;
      Vector3 new_position = Vector3Scale(
        Vector3Normalize(Vector3Add(
          camera.position, Vector3Add(
                             Vector3Scale(camera.up, dy),
                             Vector3Scale(Vector3CrossProduct(camera.up, Vector3Normalize(camera.position)), dx)
                           )
        )),
        std::sqrt(8)
      );
      Vector3 new_up = Vector3CrossProduct(
        Vector3Normalize(new_position), Vector3CrossProduct(camera.up, Vector3Normalize(camera.position))
      );
      camera.position = new_position;
      camera.up = new_up;
    }
    BeginDrawing();
    ClearBackground(WHITE);
    BeginMode3D(camera);
    draw_arc(point2vec(points[0]), point2vec(points[1]), BLUE);
    for (auto&& p : points) {
      DrawSphere(point2vec(p), point_radius, point_color);
      // draw_circle(point2vec(p), 0.01f, point2vec(p), point_color);
      // DrawCircle3D(point2vec(p), 0.01f, point2vec(p), 0.0f, point_color);
    }
    if (!IsKeyDown(KEY_C)) {
      DrawSphereEx(sphere_center, sphere_radius, 64, 64, sphere_color);
    }
    EndMode3D();
    DrawFPS(10, 10);
    DrawText(std::format("rotation speed: {}", rotation_speed).c_str(), 10, 30, 30, GREEN);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
