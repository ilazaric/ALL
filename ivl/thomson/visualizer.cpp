#include "eval"
#include "gradient"
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

void draw_arrow(Vector3 from, Vector3 direction, Color color) {
  auto to = Vector3Add(from, direction);
  auto mid = Vector3Lerp(from, to, 0.75f);
  auto delta = Vector3Scale(Vector3Normalize(Vector3CrossProduct(from, direction)), Vector3Length(direction) * 0.05f);
  DrawLine3D(from, mid, color);
  DrawTriangle3D(to, Vector3Add(mid, delta), Vector3Subtract(mid, delta), color);
  DrawTriangle3D(to, Vector3Subtract(mid, delta), Vector3Add(mid, delta), color);
}

bool try_newton_fixup(std::span<point> points, double& ev) {
  auto g = gradient2(points);
  auto g2 = diff2(points);
  double up = 0.0;
  for (std::size_t i = 0; i < points.size(); ++i) up += dot(g[i], g[i]);
  double down = 0.0;
  for (std::size_t i = 0; i < points.size(); ++i)
    for (std::size_t j = 0; j < points.size(); ++j)
      for (int k = 0; k < 3; ++k)
        for (int l = 0; l < 3; ++l) down += g2[i * points.size() + j][k][l] * g[i][k] * g[j][l];
  double coef = -up / down;
  // LOG(coef);
  std::vector nxt(std::from_range, points);
  for (std::size_t i = 0; i < points.size(); ++i) nxt[i] += coef * g[i];
  auto nxt_ev = evaluate(nxt);
  // if (nxt_ev > ev - 1e-5) return false;
  for (std::size_t i = 0; i < points.size(); ++i) points[i] = nxt[i];
  ev = nxt_ev;
  normalize(points);
  return true;
}

std::vector<point> fixup(std::span<const point> points, double* last = nullptr) {
  auto g = gradient2(points);
  for (int i = 0; i < points.size(); ++i) {
    g[i] -= points[i] * dot(g[i], points[i]);
  }
  std::vector<point> copy(points.size());
  auto mix = [&](double r) -> std::span<const point> {
    for (std::size_t i = 0; i < points.size(); ++i) copy[i] = points[i] - r * g[i];
    normalize(copy);
    return copy;
  };
  double lo = 0;
  double lo_e = evaluate_assume_normed(mix(lo));
  double hi = 1; // last ? *last : 1;
  double hi_e = evaluate_assume_normed(mix(hi));
  // if (hi_e < lo_e - 1e-2) return mix(hi);
  // while (true) {
  //   double nhi = hi * 1.5;
  //   double nhi_e = evaluate_assume_normed(mix(nhi));
  //   if (nhi_e > hi_e - 1e-6) break;
  //   hi = nhi;
  //   hi_e = nhi_e;
  // }
  while ((hi - lo) > 1e-9) {
    double mid = (hi + lo) / 2;
    double mid_e = evaluate_assume_normed(mix(mid));
    if (lo_e < hi_e) hi = mid, hi_e = mid_e;
    else lo = mid, lo_e = mid_e;
  }
  // LOG(lo);
  if (last) *last = lo;
  mix(lo);
  return copy;
}

bool try_gradient_fixup(std::span<point> points, double& ev, double* last = nullptr) {
  auto nxt = fixup(points, last);
  auto nxt_ev = evaluate(nxt);
  if (nxt_ev > ev - 1e-5) return false;
  for (std::size_t i = 0; i < points.size(); ++i) points[i] = nxt[i];
  ev = nxt_ev;
  normalize(points);
  return true;
}

struct sphere_render {
  RenderTexture2D RT = LoadRenderTexture(500, 500);
  Camera camera{};
  Vector3 sphere_center{0.0, 0.0, 0.0};
  double sphere_radius = 1.0;
  Color sphere_color = ColorAlpha(RED, 0.5);
  Color point_color = GREEN;
  float point_radius = 0.01f;
  std::vector<point> points;
  double ev;
  float rotation_speed = 1.0f;
  int iteration = 0;

  explicit sphere_render(std::size_t point_count) : points(point_count) {
    for (auto& p : points) p = random_point();
    normalize(points);
    ev = evaluate(points);
    camera.position = (Vector3){0.0f, 2.0f, 2.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
  }

  void render() {
    BeginTextureMode(RT);
    ClearBackground(WHITE);
    BeginMode3D(camera);
    // draw_arc(point2vec(points[0]), point2vec(points[1]), BLUE);
    if (!IsKeyDown(KEY_P)) {
      auto g = gradient2(points);
      for (std::size_t i = 0; i < points.size(); ++i) {
        draw_arrow(point2vec(points[i]), point2vec(g[i]), BLUE);
      }
    }
    for (auto&& p : points) {
      DrawSphereEx(point2vec(p), point_radius, 8, 8, point_color);
      // draw_circle(point2vec(p), 0.01f, point2vec(p), point_color);
      // DrawCircle3D(point2vec(p), 0.01f, point2vec(p), 0.0f, point_color);
    }
    if (!IsKeyDown(KEY_C)) {
      DrawSphereEx(sphere_center, sphere_radius, 64, 64, sphere_color);
    }
    EndMode3D();
    EndTextureMode();
  }

  void handle_input(float dt) {
    auto iterate = [&](std::size_t num) {
      while (num--) {
        ++iteration;
        try_gradient_fixup(points, ev);
      }
    };
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
    if (IsKeyPressed(KEY_ONE)) iterate(1);
    if (IsKeyPressed(KEY_TWO)) iterate(10);
    if (IsKeyPressed(KEY_THREE)) iterate(100);
    if (IsKeyPressed(KEY_FOUR)) iterate(1000);
  }
};

template<typename T>
struct plot_render {
  T fn;
  RenderTexture2D RT = LoadRenderTexture(500, 500);
  double hi = 1.0;
  double scale = 1.0;

  void render() {
    BeginTextureMode(RT);
    ClearBackground(WHITE);
    DrawLine(0, 250, 500, 250, GRAY);
    DrawLine(0, 375, 500, 375, RED);
    DrawLine(0, 125, 500, 125, GREEN);
    auto baseline = fn(0.0);
    auto last = baseline;
    const int delta = 1;
    for (int i = delta; i <= 500; i += delta) {
      auto curr = fn((float)i / 500 * hi);
      DrawLine(i - delta, scale * (last - baseline) + 250, i, scale * (curr - baseline) + 250, BLACK);
      last = curr;
    }
    DrawText(std::format("hi: {}", hi).c_str(), 10, 10, 30, GREEN);
    DrawText(std::format("scale: {}", hi).c_str(), 10, 40, 30, GREEN);
    EndTextureMode();
  }

  void handle_input(float dt) {
    if (IsKeyPressed(KEY_UP)) scale *= 2;
    if (IsKeyPressed(KEY_DOWN)) scale /= 2;
    if (IsKeyPressed(KEY_RIGHT)) hi /= 2;
    if (IsKeyPressed(KEY_LEFT)) hi *= 2;
  }
};

void draw_texture(Texture2D texture, Vector2 position) {
  DrawTextureRec(texture, Rectangle{0.0f, 0.0f, (float)texture.width, -(float)texture.height}, position, WHITE);
}

int ivl_main(std::size_t point_count) {
  const int screenWidth = 1000;
  const int screenHeight = 1000;
  // SetConfigFlags(FLAG_WINDOW_HIGHDPI);
  InitWindow(screenWidth, screenHeight, "visualizer");
  sphere_render sr(point_count);
  plot_render pr{[&](double x) {
    auto g = gradient2(sr.points);
    for (std::size_t i = 0; i < sr.points.size(); ++i) g[i] = sr.points[i] - g[i] * x;
    return evaluate(g);
  }};

  bool active = true;
  SetTargetFPS(30);
  while (!WindowShouldClose()) {
    float dt = GetFrameTime();
    if (IsKeyPressed(KEY_TAB)) active = !active;
    // if (active)
      sr.handle_input(dt);
    // else
      pr.handle_input(dt);
    sr.render();
    pr.render();
    BeginDrawing();
    ClearBackground(WHITE);
    draw_texture(sr.RT.texture, {0, 0});
    draw_texture(pr.RT.texture, {500, 0});
    // DrawTexture(sr.RT.texture, 0, 0, WHITE);
    // DrawTexture(pr.RT.texture, 500, 0, WHITE);
    DrawFPS(10, 10);
    DrawText(std::format("rotation speed: {}", sr.rotation_speed).c_str(), 10, 30, 30, GREEN);
    DrawText(std::format("iteration: {}", sr.iteration).c_str(), 10, 60, 30, GREEN);
    DrawText(std::format("eval: {}", sr.ev).c_str(), 10, 90, 30, GREEN);
    EndDrawing();
  }
  CloseWindow();
  return 0;
}
