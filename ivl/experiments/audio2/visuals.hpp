#pragma once

#include <ivl/logger>
#include <raylib/raylib.h>
#include <algorithm>
#include <format>
#include <span>
#include <vector>

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

    const int axis_font_size = 20;
    const int value_font_size = 20;
    const int padding_width_left = 5 + MeasureText("20Hz", value_font_size) / 2;
    const int padding_width_right = 5 + MeasureText("20000Hz", value_font_size) / 2;
    const int padding_height = 5;
    const int plot_width = screen_width - padding_width_left - padding_width_right;
    const int plot_height = screen_height - padding_height * 2 - axis_font_size - value_font_size;
    const int plot_x = padding_width_left;
    const int plot_y = padding_height + axis_font_size + value_font_size;
    std::vector<Vector2> points;
    double my = std::ranges::max(input);
    double min_log_freq = std::log(min_freq);
    double max_log_freq = std::log(max_freq);
    for (size_t i = 0; i < input.size() / 2; ++i) {
      double freq = sample_rate * (double)i / (double)input.size();
      if (freq < min_freq) continue;
      if (freq > max_freq) break;
      double log_freq = std::log(freq);
      auto x = plot_x + (double)plot_width * (log_freq - min_log_freq) / (max_log_freq - min_log_freq);
      auto y = plot_y + (double)plot_height * double(input[i]) / my;
      points.emplace_back(x, y);
    }
    RenderTexture2D RT = LoadRenderTexture(screen_width, screen_height);
    RenderTexture2D textRT = LoadRenderTexture(screen_width, screen_height);
    {
      contract_assert(points.size() < (1ull << 31));

      BeginTextureMode(textRT);
      auto draw_text_centered = [&](const char* text, int x, int y, int font_size) {
        auto len = MeasureText(text, font_size);
        DrawText(text, x - len / 2, textRT.texture.height - y - font_size / 2, font_size, BLACK);
      };
      for (double freq : {20.0, 50.0, 100.0, 200.0, 500.0, 1'000.0, 2'000.0, 5'000.0, 10'000.0, 20'000.0}) {
        double log_freq = std::log(freq);
        auto x = plot_x + (double)plot_width * (log_freq - min_log_freq) / (max_log_freq - min_log_freq);
        auto text = std::format("{}Hz", freq);
        auto len = MeasureText(text.c_str(), value_font_size);
        draw_text_centered(text.c_str(), x, plot_y - value_font_size / 2, value_font_size);
      }
      draw_text_centered("frequency", screen_width / 2, padding_height + axis_font_size / 2, axis_font_size);
      EndTextureMode();

      BeginTextureMode(RT);
      ClearBackground(WHITE);
      DrawTexture(textRT.texture, 0, 0, WHITE);
      for (double freq : {20.0, 50.0, 100.0, 200.0, 500.0, 1'000.0, 2'000.0, 5'000.0, 10'000.0, 20'000.0}) {
        double log_freq = std::log(freq);
        auto x = plot_x + (double)plot_width * (log_freq - min_log_freq) / (max_log_freq - min_log_freq);
        DrawLineDashed({(float)x, (float)plot_y}, {(float)x, (float)(plot_y + plot_height)}, 10, 10, GRAY);
      }
      DrawLine(plot_x, plot_y, plot_x + plot_width, plot_y, BLACK);
      DrawLine(plot_x, plot_y + plot_height, plot_x, plot_y, BLACK);
      DrawLine(plot_x, plot_y + plot_height, plot_x + plot_width, plot_y + plot_height, BLACK);
      DrawLine(plot_x + plot_width, plot_y, plot_x + plot_width, plot_y + plot_height, BLACK);
      DrawLineStrip(points.data(), (int)points.size(), BLUE);
      // DrawLine(0, 0, screen_width, screen_height, GRAY);
      EndTextureMode();
    }
    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(PINK); // to see mistakes
      DrawTexture(RT.texture, 0, 0, WHITE);
      EndDrawing();
    }
    CloseWindow();
    UnloadRenderTexture(RT);
  }
}
