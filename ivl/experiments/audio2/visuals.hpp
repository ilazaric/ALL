#pragma once

#include <ivl/logger>
#include <raylib/raylib.h>
#include <raylib/raymath.h>
#include <algorithm>
#include <cmath>
#include <format>
#include <span>
#include <vector>

struct plot {
  RenderTexture2D RT;
  RenderTexture2D textRT;

  int axis_font_size = 0;
  int value_font_size = 0;

  int padding_left = 0;
  int padding_right = 0;
  int padding_top = 0;
  int padding_bottom = 0;
  void set_all_padding(int v) {
    padding_left = v;
    padding_right = v;
    padding_top = v;
    padding_bottom = v;
  }

  bool render_x_axis_label = false;
  std::string x_axis_label;
  bool render_x_axis_values = false;
  std::vector<std::pair<double, std::string>> x_axis_values;

  std::pair<double, double> x_range{};
  std::pair<double, double> y_range{};

  struct point_sequence {
    std::vector<Vector2> points;
    Color color = PINK;
  };
  std::vector<point_sequence> sequences;

  plot(size_t width, size_t height) : RT(LoadRenderTexture(width, height)), textRT(LoadRenderTexture(width, height)) {}

  ~plot() {
    UnloadRenderTexture(RT);
    UnloadRenderTexture(textRT);
  }

  void internal_render_fail() {
    BeginTextureMode(RT);
    ClearBackground(PINK);
    EndTextureMode();
  }

  void internal_render() {
    contract_assert(!render_x_axis_label || axis_font_size > 0);
    contract_assert(!render_x_axis_values || value_font_size > 0);
    contract_assert(std::isfinite(x_range.first));
    contract_assert(std::isfinite(x_range.second));
    contract_assert(std::isfinite(y_range.first));
    contract_assert(std::isfinite(y_range.second));
    contract_assert(x_range.first < x_range.second);
    contract_assert(y_range.first < y_range.second);
    // cant be bothered with figuring out appropriate padding in general
    contract_assert(!render_x_axis_values || !x_axis_values.empty() && x_axis_values.front().first == x_range.first);
    contract_assert(!render_x_axis_values || !x_axis_values.empty() && x_axis_values.back().first == x_range.second);
    const int current_padding_left =
      padding_left + (render_x_axis_values && !x_axis_values.empty()
                        ? MeasureText(x_axis_values.front().second.c_str(), value_font_size) / 2
                        : 0);
    const int current_padding_right =
      padding_right + (render_x_axis_values && !x_axis_values.empty()
                         ? MeasureText(x_axis_values.back().second.c_str(), value_font_size) / 2
                         : 0);
    const int current_padding_top = padding_top;
    const int current_padding_bottom =
      padding_bottom + (render_x_axis_label ? axis_font_size : 0) + (render_x_axis_values ? value_font_size : 0);
    const int plot_width = RT.texture.width - current_padding_left - current_padding_right;
    const int plot_height = RT.texture.height - current_padding_bottom - current_padding_top;
    const int plot_x = current_padding_left;
    const int plot_y = current_padding_bottom;
    if (plot_width <= 0) return internal_render_fail();
    if (plot_height <= 0) return internal_render_fail();

    BeginTextureMode(textRT);
    ClearBackground(WHITE);
    auto draw_text_centered = [&](const std::string& text, int x, int y, int font_size) {
      auto len = MeasureText(text.c_str(), font_size);
      DrawText(text.c_str(), x - len / 2, RT.texture.height - y - font_size / 2, font_size, BLACK);
    };
    if (render_x_axis_values) {
      for (auto&& [real_x, text] : x_axis_values) {
        auto x = plot_x + (double)plot_width * (real_x - x_range.first) / (x_range.second - x_range.first);
        draw_text_centered(text, x, plot_y - value_font_size / 2, value_font_size);
      }
    }
    if (render_x_axis_label)
      draw_text_centered(x_axis_label, RT.texture.width / 2, padding_bottom + axis_font_size / 2, axis_font_size);
    EndTextureMode();

    BeginTextureMode(RT);
    ClearBackground(WHITE);
    DrawTexture(textRT.texture, 0, 0, WHITE);
    if (render_x_axis_values) {
      for (auto&& [real_x, _] : x_axis_values) {
        auto x = plot_x + (double)plot_width * (real_x - x_range.first) / (x_range.second - x_range.first);
        DrawLineDashed({(float)x, (float)plot_y}, {(float)x, (float)(plot_y + plot_height)}, 10, 10, GRAY);
      }
    }
    DrawLine(plot_x, plot_y, plot_x + plot_width, plot_y, BLACK);
    DrawLine(plot_x, plot_y + plot_height, plot_x, plot_y, BLACK);
    DrawLine(plot_x, plot_y + plot_height, plot_x + plot_width, plot_y + plot_height, BLACK);
    DrawLine(plot_x + plot_width, plot_y, plot_x + plot_width, plot_y + plot_height, BLACK);
    for (auto&& sequence : sequences) {
      auto color = sequence.color;
      auto&& points = sequence.points;
      if (points.empty()) continue;
      for (auto&& window : points | std::views::slide(2)) {
        auto it = window.begin();
        auto start = *it;
        ++it;
        auto end = *it;
        // clamping a line segment to a rectangle, skipping if no intersection
        if (start.x <= x_range.first && end.x <= x_range.first) continue;
        if (start.x >= x_range.second && end.x >= x_range.second) continue;
        if (start.y <= y_range.first && end.y <= y_range.first) continue;
        if (start.y >= y_range.second && end.y >= y_range.second) continue;
        if (start.x < x_range.first) start = Vector2Lerp(start, end, (x_range.first - start.x) / (end.x - start.x));
        if (start.x > x_range.second) start = Vector2Lerp(start, end, (x_range.second - start.x) / (end.x - start.x));
        if (end.x < x_range.first) end = Vector2Lerp(end, start, (x_range.first - end.x) / (start.x - end.x));
        if (end.x > x_range.second) end = Vector2Lerp(end, start, (x_range.second - end.x) / (start.x - end.x));
        if (start.y <= y_range.first && end.y <= y_range.first) continue;
        if (start.y >= y_range.second && end.y >= y_range.second) continue;
        if (start.y < y_range.first) start = Vector2Lerp(start, end, (y_range.first - start.y) / (end.y - start.y));
        if (start.y > y_range.second) start = Vector2Lerp(start, end, (y_range.second - start.y) / (end.y - start.y));
        if (end.y < y_range.first) end = Vector2Lerp(end, start, (y_range.first - end.y) / (start.y - end.y));
        if (end.y > y_range.second) end = Vector2Lerp(end, start, (y_range.second - end.y) / (start.y - end.y));
        // now within box, need to translate to texture coords
        DrawLineV(
          {
            (start.x - x_range.first) / (x_range.second - x_range.first) * plot_width + plot_x,
            (start.y - y_range.first) / (y_range.second - y_range.first) * plot_width + plot_y,
          },
          {
            (end.x - x_range.first) / (x_range.second - x_range.first) * plot_width + plot_x,
            (end.y - y_range.first) / (y_range.second - y_range.first) * plot_width + plot_y,
          },
          color
        );
      }
    }
    EndTextureMode();
  }
};

void stft_visualise(std::span<const double> input, double sample_rate) {
  contract_assert(!input.empty());
  std::vector<size_t> indices(std::from_range, std::views::iota(0ull, input.size()));
  std::ranges::sort(indices, std::ranges::greater{}, [&](size_t i) { return input[i]; });
  for (size_t i = 0; i < indices.size() && i < 200; ++i) {
    LOG(i, indices[i], input[indices[i]]);
  }
  {
    const int screen_width = 1000;
    const int screen_height = 1000;
    InitWindow(screen_width, screen_height, "visualiser");
    SetTargetFPS(30);

    plot p(screen_width, screen_height);
    p.axis_font_size = 20;
    p.value_font_size = 20;
    p.set_all_padding(5);
    p.render_x_axis_label = true;
    p.x_axis_label = "frequency";
    p.render_x_axis_values = true;
    for (double freq : {20.0, 50.0, 100.0, 200.0, 500.0, 1'000.0, 2'000.0, 5'000.0, 10'000.0, 20'000.0}) {
      p.x_axis_values.emplace_back(std::log(freq), std::format("{}Hz", freq));
    }
    const double min_log_freq = std::log(min_freq);
    const double max_log_freq = std::log(max_freq);
    const double my = std::ranges::max(input);
    p.x_range = {min_log_freq, max_log_freq};
    p.y_range = {0, my};
    {
      p.sequences.emplace_back();
      p.sequences[0].color = BLUE;
      auto& points = p.sequences[0].points;
      for (size_t i = 0; i < input.size() / 2; ++i) {
        double freq = sample_rate * (double)i / (double)input.size();
        if (freq < min_freq / 2) continue;
        if (freq > max_freq * 2) break;
        double log_freq = std::log(freq);
        points.emplace_back(log_freq, input[i]);
      }
    }
    p.internal_render();

    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(GREEN); // to see mistakes
      DrawTexture(p.RT.texture, 0, 0, WHITE);
      EndDrawing();
    }
    CloseWindow();
  }
}
