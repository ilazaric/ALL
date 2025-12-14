#include "raylib.h"
#include "raymath.h"

#include <format>
#include <sstream>
#include <vector>

template <>
struct std::formatter<Vector2> : std::formatter<std::string> {
  auto format(Vector2 v, format_context& ctx) const {
    return formatter<string>::format(std::format("({}, {})", v.x, v.y), ctx);
  }
};

int main(void) {
  // const int W = 1960, H = 1080;
  // const int W = 800;
  // const int H = 600;
  const int W = GetMonitorWidth(0) / 2;
  const int H = GetMonitorHeight(0) / 2;
  InitWindow(W, H, "! blatruc !");
  SetTargetFPS(60);

  const int CircleRadius = 500;
  auto CircleRT = LoadRenderTexture(CircleRadius * 2.4, CircleRadius * 2.4);
  BeginTextureMode(CircleRT);
  {
    ClearBackground(BLANK);
    // DrawCircle(CircleRadius, CircleRadius, CircleRadius, WHITE);
    DrawCircleSector(
      Vector2{(float)(CircleRT.texture.width / 2.), (float)(CircleRT.texture.height / 2.)}, CircleRadius * 0.9, 0, 360,
      360, WHITE
    );
    int edge_step = 25;
    for (int i = 1; i <= edge_step; ++i)
      DrawRing(
        Vector2{(float)(CircleRT.texture.width / 2.), (float)(CircleRT.texture.height / 2.)},
        CircleRadius * (0.9 + 0.2 * (i - 1) / edge_step), CircleRadius * (0.9 + 0.2 * i / edge_step), 0, 360, 360,
        ColorAlpha(WHITE, 1 - (i - 1) * 1.f / edge_step)
      );
  }
  EndTextureMode();
  // SetTextureFilter(CircleRT.texture, TEXTURE_FILTER_POINT);

  auto draw_circle_fast = [&](Vector2 pos, float radius, Color color) {
    DrawTextureEx(
      CircleRT.texture, Vector2Subtract(pos, Vector2{radius * 1.2f, radius * 1.2f}), 0, radius / CircleRadius, color
    );
  };

  std::vector<Vector2> mouse_positions;
  std::vector<double> frame_times;
  std::vector<double> frame_times_accumulated;
  const Color head_color = GREEN;
  const Color tail_color = RED;
  const float max_total_time = 5; // seconds

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_R)) {
      mouse_positions.clear();
      frame_times.clear();
      frame_times_accumulated.clear();
    }

    mouse_positions.emplace_back(GetMousePosition());
    frame_times.emplace_back(GetFrameTime() / max_total_time);
    frame_times_accumulated.emplace_back(0);

    {
      float total = 0;
      for (size_t i = mouse_positions.size() - 1; i + 1; --i) {
        if (total > 1) {
          mouse_positions.erase(mouse_positions.begin(), mouse_positions.begin() + i + 1);
          frame_times.erase(frame_times.begin(), frame_times.begin() + i + 1);
          frame_times_accumulated.erase(frame_times_accumulated.begin(), frame_times_accumulated.begin() + i + 1);
          break;
        }
        frame_times_accumulated[i] = total;
        total += frame_times[i];
      }
    }

    // --- draw ----------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);

    {
      for (size_t i = 0; i < mouse_positions.size(); ++i) {
        // DrawPixelV(mouse_positions[i], ColorLerp(head_color, tail_color, total / max_total_time));
        const float staleness = frame_times_accumulated[i];
        const float alpha = (1 - staleness) * (1 - staleness);
        const Color color = ColorAlpha(ColorLerp(head_color, tail_color, staleness), alpha);
        // DrawCircleV(mouse_positions[i], staleness * 100, color);
        const float max_radius = 100;
        const float scaling = staleness * max_radius / CircleRadius;
        draw_circle_fast(mouse_positions[i], max_radius * staleness, color);
        // DrawTextureEx(CircleRT.texture, Vector2Subtract(mouse_positions[i], Vector2{CircleRadius*scaling,
        // CircleRadius*scaling}), 0, scaling, color);
      }
    }

    // DrawPixelV

    auto draw_text = [h = 30, c = 10]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) mutable {
      DrawText(std::format(fmt, std::forward<Ts>(args)...).c_str(), 10, h, c, WHITE);
      h += c;
    };
    DrawFPS(10, 10);
    draw_text("hello world");
#define X(...) draw_text(#__VA_ARGS__ ": {}", __VA_ARGS__)
    X(GetScreenWidth());
    X(GetScreenHeight());
    X(GetRenderWidth());
    X(GetRenderHeight());
    // X(GetMonitorCount());
    // X(GetCurrentMonitor());
    // X(GetMonitorPosition(0));
    X(GetMonitorWidth(0));
    X(GetMonitorHeight(0));
    // X(GetMonitorPhysicalWidth(0));
    // X(GetMonitorPhysicalHeight(0));
    // X(GetMonitorRefreshRate(0));
    X(GetWindowPosition());
    // X(GetWindowScaleDPI());
    // X(GetMonitorName(0));
    // X(GetClipboardText()); // weird?
    X(IsCursorHidden());
    X(IsCursorOnScreen());
    X(GetMouseX());
    X(GetMouseY());
    X(mouse_positions.size());
#undef X
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
