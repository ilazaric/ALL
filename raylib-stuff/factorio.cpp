#ifdef NDEBUG
#undef NDEBUG
#endif

#include "raylib.h"
#include "raymath.h"

#include <algorithm>
#include <boost/unordered/unordered_flat_map.hpp>
#include <cassert>
#include <filesystem>
#include <format>
#include <functional>
#include <print>
#include <random>
#include <ranges>
#include <span>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "absl/functional/any_invocable.h"

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

#define ivl_assert(cond, ...)                                                                                          \
  do {                                                                                                                 \
    if (!(cond)) {                                                                                                     \
      std::cout << "ERROR: failed condition: " #cond << std::endl;                                                     \
      LOG(__VA_ARGS__);                                                                                                \
      assert(false);                                                                                                   \
    }                                                                                                                  \
  } while (0)

enum class dir { N, E, S, W };

int dx(dir d) {
  switch (d) {
    using enum dir;
  case N:
    return 0;
  case E:
    return 1;
  case S:
    return 0;
  case W:
    return -1;
  }
}

int dy(dir d) {
  switch (d) {
    using enum dir;
  case N:
    return -1;
  case E:
    return 0;
  case S:
    return 1;
  case W:
    return 0;
  }
}

struct global_game_state {
  int window_width;
  int window_height;
  // Vector2 render_position;
  // float scale;
  // float rotation;
  // Color tint;
} global_game_state;

struct game_entity {
  virtual void logic_tick() {}
  virtual void render(int x, int y) const {}
};

void draw_texture(Texture2D texture, int x, int y) {
  if (x > global_game_state.window_width ||  //
      y > global_game_state.window_height || //
      x + texture.width < 0 ||               //
      y + texture.height < 0)
    return;
  DrawTexture(texture, x, y, WHITE);
}

struct animation {
  std::span<const Texture2D> textures;
  size_t idx;

  int width() const {
    ivl_assert(!textures.empty());
    return textures[0].width;
  }
  int height() const {
    ivl_assert(!textures.empty());
    return textures[0].height;
  }
  size_t length() const { return textures.size(); }

  void render(int x, int y) const { draw_texture(textures[idx], x, y); }

  void logic_tick() {
    if (++idx == textures.size()) idx = 0;
  }
};

// struct item {
//   // nullptr can be meaningful
//   const Texture2D* texture;

//   bool empty() const { return texture == nullptr; }

//   void render(int x, int y) const {
//     if (texture) draw_texture(*texture, x, y);
//   }
// };

// struct belt {
//   // input/output
//   enum class IO { I, O };
//   struct FOO {
//     IO io;
//     std::array<item, 8> slots;
//   };
//   std::array<FOO, 4> dirs;
//   item center;
//   dir input_dir;
//   dir output_dir;
//   std::array<item, 8> slots;
//   int slot_head = 0;

//   void render(int x, int y) const {
//     a.render(x, y);
//     for (int i = 0; i < slots.size(); ++i) slots[i].render(x, y);
//   }

//   void logic_tick() {
//     a.logic_tick();
//     // TODO
//   }
// };

struct Vector2i {
  int x;
  int y;
};

// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .
// . . . . . . . . . .

// int -> int -> T
template <typename T>
struct hashmap2 {
  boost::unordered_flat_map<uint64_t, T> underlying;

  static uint64_t encode(int x, int y) { return ((uint64_t)(uint32_t)x) << 32 | ((uint64_t)(uint32_t)y); }

  decltype(auto) operator[](int x, int y) { return underlying[encode(x, y)]; }
  decltype(auto) operator[](Vector2i v) { return underlying[encode(v.x, v.y)]; }
};

// hashmap2<callbacks> input_callbacks;
// hashmap2<callbacks> output_callbacks;

void occupy(int x, int y);
void release(int x, int y);

struct moving_item {
  const Texture2D* texture;
  Vector2i start;
  Vector2i end;
  int step;
  int step_count;

  void render(int x, int y) const;

  void logic_tick() {
    if (step != step_count) {
      ++step;
      return;
    }
  }
};

struct magic_container {
  std::vector<absl::AnyInvocable<void()>> logic_callbacks;
  size_t unused_count;
};

struct render_texture {
  // struct api {
  //   render_texture& RT;
  //   void draw_texture();
  // };
  
  RenderTexture2D RT;

  render_texture(int w, int h) : RT(LoadRenderTexture(w, h)) {}

  render_texture(const render_texture&) = delete;
  render_texture(render_texture&& o) = delete;
  render_texture& operator=(const render_texture&) = delete;
  render_texture& operator=(render_texture&&) = delete;

  void resize(int w, int h) {
    UnloadRenderTexture(RT);
    RT = LoadRenderTexture(w, h);
  }

  void render_into(auto&& fn) {
    BeginTextureMode(RT);
    FWD(fn)();
    EndTextureMode();
  }

  Texture2D get_texture() const { return RT.texture; }

  void render_out() const {
    DrawTextureRec(RT.texture, {0.f, 0.f, (float)RT.texture.width, -(float)RT.texture.height}, {0.f, 0.f}, WHITE);
  }

  ~render_texture() { UnloadRenderTexture(RT); }
};

struct world {
  int tile_size = 10;
  std::vector<Texture2D> tile_textures;
  std::unordered_map<int64_t, std::unordered_map<int64_t, uint64_t>> ground_tiles;

  std::vector<absl::AnyInvocable<void()>> logic_callbacks;
  std::vector<size_t> unused_callback_slots;

  render_texture RT{global_game_state.window_width, global_game_state.window_height};

  size_t emplace_logic_callback(absl::AnyInvocable<void()>&& callback) {
    if (unused_callback_slots.empty()) {
      logic_callbacks.emplace_back(std::move(callback));
      return logic_callbacks.size() - 1;
    } else {
      auto idx = unused_callback_slots.back();
      unused_callback_slots.pop_back();
      logic_callbacks[idx] = std::move(callback);
      return idx;
    }
  }

  void remove_logic_callback(size_t idx) {
    logic_callbacks[idx] = {};
    unused_callback_slots.push_back(idx);
  }

  size_t get_tile(int64_t x, int64_t y) const {
    auto it1 = ground_tiles.find(x);
    if (it1 == ground_tiles.end()) return 0;
    auto it2 = it1->second.find(y);
    if (it2 == it1->second.end()) return 0;
    return it2->second;
  }

  void set_tile(int64_t x, int64_t y, uint64_t t) { ground_tiles[x][y] = t; };

  void move_view(int start_x, int start_y) {
    RT.render_into([&] {
      int off_x = start_x % tile_size;
      int off_y = start_y % tile_size;
      int idx_x = start_x / tile_size;
      int idx_y = start_y / tile_size;
      if (off_x < 0) {
        off_x += tile_size;
        --idx_x;
      }
      if (off_y < 0) {
        off_y += tile_size;
        --idx_y;
      }
      // LOG(width, height, tile_size);
      for (int x = -1; x <= RT.texture.width / tile_size; ++x)
        for (int y = -1; y <= RT.texture.height / tile_size; ++y) {
          // LOG(-off_x + x * tile_size, -off_y + y * tile_size);
          draw_texture(
                       tile_textures[get_tile(idx_x + x, idx_y + y)], //
                       -off_x + x * tile_size,                        //
                       -off_y + y * tile_size
                       );
        }
    });
  }

  void render_ground(int start_x, int start_y, int width, int height) {
    int off_x = start_x % tile_size;
    int off_y = start_y % tile_size;
    int idx_x = start_x / tile_size;
    int idx_y = start_y / tile_size;
    if (off_x < 0) {
      off_x += tile_size;
      --idx_x;
    }
    if (off_y < 0) {
      off_y += tile_size;
      --idx_y;
    }
    // LOG(width, height, tile_size);
    for (int x = -1; x <= width / tile_size; ++x)
      for (int y = -1; y <= height / tile_size; ++y) {
        // LOG(-off_x + x * tile_size, -off_y + y * tile_size);
        draw_texture(
          tile_textures[get_tile(idx_x + x, idx_y + y)], //
          -off_x + x * tile_size,                        //
          -off_y + y * tile_size
        );
      }
  }

  void game_logic_tick() {
    for (auto&& callback : logic_callbacks)
      if (callback) callback();
  }
};

int main(void) {
  const int W = GetMonitorWidth(0) / 2;
  const int H = GetMonitorHeight(0) / 2;
  global_game_state.window_width = W;
  global_game_state.window_height = H;
  InitWindow(W, H, "FACTORIO CLONE");
  SetTargetFPS(60);

  world w;
  LOG(w.tile_size);

  [&] {
    std::cout << "Finding tiles ..." << std::endl;
    std::vector<std::filesystem::path> files;
    for (auto&& entry : std::filesystem::directory_iterator("assets/tiles")) files.push_back(entry.path());
    std::ranges::sort(files);
    for (auto&& file : files) {
      LOG(file);
      Image img = LoadImage(file.native().c_str());
      Texture2D tex = LoadTextureFromImage(img);
      ivl_assert(tex.width == w.tile_size && tex.height == w.tile_size, file, tex.width, tex.height);
      UnloadImage(img);
      w.tile_textures.push_back(tex);
    }
    {
      auto RT = LoadRenderTexture(w.tile_size, w.tile_size);
      BeginTextureMode(RT);
      ClearBackground(GRAY);
      EndTextureMode();
      w.tile_textures.push_back(RT.texture);
    }
    std::cout << "xxxxxxxxxxxxx Done finding tiles." << std::endl;
  }();

  LOG(w.tile_size);
  std::cout << "tile_size: " << w.tile_size << std::endl;

  Texture2D mine;
  {
    auto RT = LoadRenderTexture(w.tile_size * 3, w.tile_size * 3);
    BeginTextureMode(RT);
    ClearBackground(ColorAlpha(WHITE, 0));
    // DrawLine(1, 1, 1, tile_size * 3 - 1, BLUE);
    // DrawLine(1, 1, tile_size * 3 - 1, 1, BLUE);
    DrawLine(0, 0, w.tile_size * 3, w.tile_size * 3, BLUE);
    DrawLine(0, w.tile_size * 3, w.tile_size * 3, 0, BLUE);
    DrawLine(w.tile_size * 1.5, w.tile_size * 1.5, w.tile_size * 1.5, 0, BLUE);
    EndTextureMode();
    mine = RT.texture;
  };

  for (int i = 0; i < 100; ++i)
    for (int j = 0; j < 100; ++j) {
      w.set_tile(i * 20, j * 20, 1);
      w.set_tile(i * 20 + 1, j * 20, 1);
      w.set_tile(i * 20 - 1, j * 20, 1);
      w.set_tile(i * 20, j * 20 + 1, 1);
      w.set_tile(i * 20, j * 20 - 1, 1);
    }

  int64_t view_x = 0;
  int64_t view_y = 0;
  int64_t iron = 0;

  auto RT = LoadRenderTexture(W, H);

  while (!WindowShouldClose()) {
    if (IsKeyDown(KEY_W)) --view_y;
    if (IsKeyDown(KEY_A)) --view_x;
    if (IsKeyDown(KEY_S)) ++view_y;
    if (IsKeyDown(KEY_D)) ++view_x;

    w.game_logic_tick();

    // --- draw ----------------------------------------------------------
    // BeginDrawing();
    BeginTextureMode(RT);
    ClearBackground(BLACK);

    w.render_ground(view_x, view_y, W, H);

    // // render machines
    // DrawTexture(mine, -view_x, -view_y, WHITE);
    // DrawLine(1, 1, 1, 50, PINK);
    // DrawLine(1, 1, 50, 1, PINK);

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
    // EndDrawing();
    // DrawTextureEx(w.tile_textures[0], {100, 100}, 0, 10, WHITE);
    EndTextureMode();

    BeginDrawing();
    // draw_texture_correct(RT.texture, 0, 0, WHITE);
    DrawTextureRec(RT.texture, {0.f, 0.f, (float)RT.texture.width, -(float)RT.texture.height}, {0.f, 0.f}, WHITE);
    // DrawTextureEx(w.tile_textures[0], {}, 0, 10, WHITE);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
