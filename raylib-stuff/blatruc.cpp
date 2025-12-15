#ifdef NDEBUG
#undef NDEBUG
#endif

#include "raylib.h"
#include "raymath.h"

#include <format>
#include <sstream>
#include <vector>
#include <random>
#include <cassert>
#include <print>
#include <span>

template <>
struct std::formatter<Vector2> : std::formatter<std::string> {
  auto format(Vector2 v, format_context& ctx) const {
    return formatter<string>::format(std::format("({}, {})", v.x, v.y), ctx);
  }
};

/*
  
    .
   / \
  .   .
   \ /
    .
    
 */

std::mt19937 gen{std::random_device()()};

std::vector<Vector2> lightning(Vector2 start, Vector2 end//, int depth = 0
                               ) {
  const auto dist2 = Vector2DistanceSqr(start, end);
  if (dist2 < 25)
    return {start, end};

  float num = std::uniform_real_distribution<>{-0.5, 0.5}(gen);
  auto delta = Vector2Subtract(start, end); // 1.73
  auto rot = Vector2{delta.y, -delta.x};
  auto shift = Vector2Scale(rot, num / 1.73);
  auto midpoint = Vector2Add(Vector2Add(end, Vector2Scale(delta, 0.5)), shift);

  // std::println("{} {} -> {} ({})", start, end, midpoint, depth);

  // {
  //   auto sdist2 = Vector2DistanceSqr(start, midpoint);
  //   auto edist2 = Vector2DistanceSqr(midpoint, end);
  //   assert(abs(sdist2 - edist2) < 1e-3);
  //   assert(sdist2 < dist2 / 1.73);
  // }

  auto first = lightning(start, midpoint//, depth+1
                         );
  auto second = lightning(midpoint, end//, depth+1
                          );
  first.pop_back();
  first.insert(first.end(), second.begin(), second.end());
  // std::println("end {}", depth);
  return first;
}

std::vector<Vector2> lightning2(Vector2 start, Vector2 end) {
  std::vector<Vector2> stored{start};
  std::vector<Vector2> queued{end};
  while (!queued.empty()) {
    end = queued.back();
    start = stored.back();
    const auto dist2 = Vector2DistanceSqr(start, end);
    if (dist2 < 25) {
      stored.push_back(end);
      queued.pop_back();
      continue;
    }

    float num = std::uniform_real_distribution<>{-0.5, 0.5}(gen);
    auto delta = Vector2Subtract(start, end); // 1.73
    auto rot = Vector2{delta.y, -delta.x};
    auto shift = Vector2Scale(rot, num / 1.73);
    auto midpoint = Vector2Add(Vector2Add(end, Vector2Scale(delta, 0.5)), shift);
    queued.push_back(midpoint);
  }
  return stored;
}

float sl_choice() { return std::uniform_real_distribution<>{-0.5, 0.5}(gen); }

std::vector<float> structured_lightning_choices(size_t count) {
  std::vector<float> ret(count);
  for (auto& el : ret) el = sl_choice();
  return ret;
}

std::vector<Vector2> structured_lightning(Vector2 start, Vector2 end, std::span<const float> choices) {
  if (choices.empty()) return {start, end};
  float num = choices[0];
  choices = choices.subspan(1);
  auto delta = Vector2Subtract(start, end); // 1.73
  auto rot = Vector2{delta.y, -delta.x};
  auto shift = Vector2Scale(rot, num / 1.73);
  auto midpoint = Vector2Add(Vector2Add(end, Vector2Scale(delta, 0.5)), shift);
  auto first = structured_lightning(start, midpoint, choices.subspan(0, choices.size()/2));
  auto second = structured_lightning(midpoint, end, choices.subspan(choices.size()/2));
  first.pop_back();
  first.insert(first.end(), second.begin(), second.end());
  return first;
}

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
    // This can't be replaced with BLANK, BLANK is transparent BLACK,
    // the edges inherit a bit of BLACK for some reason.
    ClearBackground(ColorAlpha(WHITE, 0));
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

  // std::vector<std::vector<Vector2>> Ls;

  struct SL {
    size_t sl_count = 127;
    std::vector<float> first = structured_lightning_choices(sl_count);
    std::vector<float> second = structured_lightning_choices(sl_count);
    std::vector<float> mix = std::vector<float>(sl_count);
    size_t mix_idx = 0;
    size_t mix_size = 8;
    float y = 320;

    void bla() {
      ++mix_idx;
      if (mix_idx == mix_size) {
        mix_idx = 0;
        first = std::move(second);
        second = structured_lightning_choices(sl_count);
      }

      float amount = (float)mix_idx / (float)mix_size;
      for (size_t i = 0; i < sl_count; ++i)
        mix[i] = Lerp(first[i], second[i], amount);
      auto L = structured_lightning(Vector2{320, 0}, Vector2{y, 400}, mix);
      DrawLineStrip(L.data(), L.size(), SKYBLUE);
    }

    // 0 1 2 3 3 2 3 3 1 2 3 3 2 3 3 

    void truc() {
      for (size_t i = 0; i < sl_count; ++i) {
        size_t d = 0;
        size_t c = i;
        size_t b = sl_count/2;
        while (c) {
          ++d;
          --c;
          if (c >= b) c -= b;
          b /= 2;
        }
        float maxstep = 0.025 * std::powf(1.7, d);
        if (abs(first[i] - second[i]) < maxstep) {
          first[i] = second[i];
          second[i] = sl_choice();
        } else {
          first[i] += first[i] < second[i] ? +maxstep : -maxstep;
        }
      }
      auto L = structured_lightning(Vector2{320, 0}, Vector2{y, 400}, first);
      DrawLineStrip(L.data(), L.size(), SKYBLUE);
    }
  };

  // size_t sl_count = 63;
  // auto first = structured_lightning_choices(sl_count);
  // auto second = structured_lightning_choices(sl_count);
  // std::vector<float> mix(sl_count);
  // auto rndy = [&] { return 320.f; return std::uniform_real_distribution<float>{160, 480}(gen); };
  // auto firsty = rndy();
  // auto secondy = rndy();
  // float mixy;
  // size_t mix_idx = 0;
  // size_t mix_size = 8;

  std::vector<SL> SLs(10);
  for (int i = 0; i < SLs.size(); ++i) SLs[i].y = 640.f * (i + 0.5) / SLs.size();

  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_R)) {
      mouse_positions.clear();
      frame_times.clear();
      frame_times_accumulated.clear();
    }

    // if (mouse_positions.empty() || frame_times.back() > 0.2/max_total_time) {
    //   mouse_positions.emplace_back(GetMousePosition());
    //   frame_times.emplace_back(0);
    //   frame_times_accumulated.emplace_back(0);
    // }
    // frame_times.back() += GetFrameTime() / max_total_time;

    // {
    //   float total = 0;
    //   for (size_t i = mouse_positions.size() - 1; i + 1; --i) {
    //     if (total > 1) {
    //       mouse_positions.erase(mouse_positions.begin(), mouse_positions.begin() + i + 1);
    //       frame_times.erase(frame_times.begin(), frame_times.begin() + i + 1);
    //       frame_times_accumulated.erase(frame_times_accumulated.begin(), frame_times_accumulated.begin() + i + 1);
    //       break;
    //     }
    //     frame_times_accumulated[i] = total;
    //     total += frame_times[i];
    //   }
    // }

    // --- draw ----------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);

    // for (auto& SL : SLs)
    //   SL.bla();
    for (auto& SL : SLs)
      SL.truc();

    // ++mix_idx;
    // if (mix_idx == mix_size) {
    //   mix_idx = 0;
    //   first = std::move(second);
    //   firsty = secondy;
    //   second = structured_lightning_choices(sl_count);
    //   secondy = rndy();
    // }

    // float amount = (float)mix_idx / (float)mix_size;
    // for (size_t i = 0; i < sl_count; ++i)
    //   mix[i] = Lerp(first[i], second[i], amount);
    // mixy = Lerp(firsty, secondy, amount);
    // auto L = structured_lightning(Vector2{320, 0}, Vector2{mixy, 400}, mix);
    // DrawLineStrip(L.data(), L.size(), SKYBLUE);

    // for (size_t i = 0; i < mouse_positions.size(); ++i) {
    //   const float life = frame_times_accumulated[i];
    //   const float scale = life < 0.3 ? life / 0.3 : life < 0.7 ? 1 : (1-life) / 0.3;
    //   draw_circle_fast(mouse_positions[i], 55 * scale, BLUE);
    // }
    // for (size_t i = 0; i < mouse_positions.size(); ++i) {
    //   const float life = frame_times_accumulated[i];
    //   const float scale = life < 0.3 ? life / 0.3 : life < 0.7 ? 1 : (1-life) / 0.3;
    //   draw_circle_fast(mouse_positions[i], 45 * scale, GOLD);
    // }

    // {
    //   Ls.emplace_back(lightning2(Vector2{320, 0}, Vector2{std::uniform_real_distribution<float>{160, 480}(gen), 400}));
    //   size_t N = 60;
    //   if (Ls.size() > N) Ls.erase(Ls.begin());
    //   for (size_t i = 0; i < Ls.size(); ++i)
    //     DrawLineStrip(Ls[i].data(), Ls[i].size(), ColorLerp(SKYBLUE, BLACK, (float)(Ls.size()-i-1) / (float)N));
    // }

    

    // {
    //   for (size_t i = 0; i < mouse_positions.size(); ++i) {
    //     // DrawPixelV(mouse_positions[i], ColorLerp(head_color, tail_color, total / max_total_time));
    //     const float staleness = frame_times_accumulated[i];
    //     const float alpha = (1 - staleness) * (1 - staleness);
    //     const Color color = ColorAlpha(ColorLerp(head_color, tail_color, staleness), alpha);
    //     // DrawCircleV(mouse_positions[i], staleness * 100, color);
    //     const float max_radius = 100;
    //     const float scaling = staleness * max_radius / CircleRadius;
    //     draw_circle_fast(mouse_positions[i], max_radius * staleness, color);
    //     // DrawTextureEx(CircleRT.texture, Vector2Subtract(mouse_positions[i], Vector2{CircleRadius*scaling,
    //     // CircleRadius*scaling}), 0, scaling, color);
    //   }
    // }

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
    // assert(false);
  }

  CloseWindow();
  return 0;
}
