#ifdef NDEBUG
#undef NDEBUG
#endif

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <format>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <sstream>
#include <unordered_map>
#include <vector>

#define IVL_LOCAL
#include <ivl/logger/logger>
#undef LOG_RAW

template <typename NS, typename CSL>
struct logger_hook {
  template <typename... Args>
  [[maybe_unused]] static decltype(auto) print(Args&&... args) {
    static_assert(NS::namecount == sizeof...(Args));
    std::cout << " FROM:" << CSL::file_name << ":" << CSL::function_name << "(" << CSL::line << "):" << std::endl;
    std::size_t index = 0;
    ( // should be easy to inline
      [](std::size_t& index, const Args& arg) {
        std::cout << " NOTE:" << NS::names[index++] << "=" << (arg) << std::endl;
      }(index, args),
      ...
    );
    return (ivl::logger::discardable_forward<Args>(args), ...);
  }
};

template <>
struct std::formatter<Vector2> : std::formatter<std::string> {
  auto format(Vector2 v, format_context& ctx) const {
    return formatter<string>::format(std::format("({}, {})", v.x, v.y), ctx);
  }
};

constexpr uint64_t tile_size = 10;

#define ivl_assert(cond, ...)                                                                                          \
  do {                                                                                                                 \
    if (!(cond)) {                                                                                                     \
      std::cout << "ERROR: failed condition: " #cond "\n";                                                             \
      LOG(__VA_ARGS__);                                                                                                \
    }                                                                                                                  \
  } while (0)

int main(void) {
  const int W = GetMonitorWidth(0) / 2;
  const int H = GetMonitorHeight(0) / 2;
  InitWindow(W, H, "FACTORIO CLONE");
  SetTargetFPS(60);

  const std::vector<Texture2D> tiles = [&] {
    std::cout << "Finding tiles ...\n";
    std::vector<Texture2D> tiles;
    std::vector<std::filesystem::path> files;
    for (auto&& entry : std::filesystem::directory_iterator("assets/tiles")) files.push_back(entry.path());
    std::ranges::sort(files);
    for (auto&& file : files) {
      LOG(file);
      Image img = LoadImage(file.native().c_str());
      Texture2D tex = LoadTextureFromImage(img);
      ivl_assert(tex.width == tile_size && tex.height == tile_size, file, tex.width, tex.height);
      UnloadImage(img);
      tiles.push_back(tex);
    }
    {
      auto RT = LoadRenderTexture(tile_size, tile_size);
      BeginTextureMode(RT);
      ClearBackground(GRAY);
      EndTextureMode();
      tiles.push_back(RT.texture);
    }
    std::cout << "Done finding tiles.\n";
    return tiles;
  }();

  std::unordered_map<int64_t, std::unordered_map<int64_t, uint64_t>> ground_tiles;

  auto get_tile = [&](int64_t x, int64_t y) -> uint64_t {
    auto it1 = ground_tiles.find(x);
    if (it1 == ground_tiles.end()) return 0;
    auto it2 = it1->second.find(y);
    if (it2 == it1->second.end()) return 0;
    return it2->second;
  };

  auto set_tile = [&](int64_t x, int64_t y, uint64_t t) { ground_tiles[x][y] = t; };

  for (int i = 0; i < 100; ++i)
    for (int j = 0; j < 100; ++j) {
      set_tile(i * 20, j * 20, 1);
      set_tile(i * 20 + 1, j * 20, 1);
      set_tile(i * 20 - 1, j * 20, 1);
      set_tile(i * 20, j * 20 + 1, 1);
      set_tile(i * 20, j * 20 - 1, 1);
    }

  int64_t view_x = 0;
  int64_t view_y = 0;
  int64_t iron = 0;

  auto render_ground = [&] {
    int64_t off_x = view_x % (int)tile_size;
    int64_t off_y = view_y % (int)tile_size;
    int64_t idx_x = view_x / (int)tile_size;
    int64_t idx_y = view_y / (int)tile_size;
    if (off_x < 0) {
      off_x += (int)tile_size;
      --idx_x;
    }
    if (off_y < 0) {
      off_y += (int)tile_size;
      --idx_y;
    }
    for (int x = -1; x <= W / (int)tile_size; ++x)
      for (int y = -1; y <= H / (int)tile_size; ++y) {
        DrawTexture(
          tiles[get_tile(idx_x + x, idx_y + y)], //
          -off_x + x * tile_size,                //
          -off_y + y * tile_size,                //
          WHITE
        );
      }
  };

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_W)) --view_y;
    if (IsKeyDown(KEY_A)) --view_x;
    if (IsKeyDown(KEY_S)) ++view_y;
    if (IsKeyDown(KEY_D)) ++view_x;

    // --- draw ----------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);

    render_ground();

    // render machines

    auto draw_text = [h = 30, c = 10]<typename... Ts>(std::format_string<Ts...> fmt, Ts&&... args) mutable {
      DrawText(std::format(fmt, std::forward<Ts>(args)...).c_str(), 10, h, c, WHITE);
      h += c;
    };
    DrawFPS(10, 10);
    draw_text("DBG:");
#define X(...) draw_text(#__VA_ARGS__ ": {}", __VA_ARGS__)
    X(GetScreenWidth());
    X(GetScreenHeight());
    X(GetRenderWidth());
    X(GetRenderHeight());
    X(GetMonitorWidth(0));
    X(GetMonitorHeight(0));
    X(GetWindowPosition());
    X(IsCursorHidden());
    X(IsCursorOnScreen());
    X(GetMouseX());
    X(GetMouseY());
    X(GetFrameTime());
    draw_text("view_{{x,y}}: {} {}", view_x, view_y);
#undef X
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
