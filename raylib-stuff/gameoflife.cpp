#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <emscripten/html5.h>
#include <algorithm>
#include <iostream>

constexpr int DESIGN_W = 1280;
constexpr int DESIGN_H = 720;
constexpr int CELL_W = 12;
constexpr int CELL_H = 12;
constexpr int DIM_W = 50;
constexpr int DIM_H = 50;

static_assert(CELL_W * DIM_W <= DESIGN_W);
static_assert(CELL_H * DIM_H <= DESIGN_H);

int main()
{
  InitWindow(DESIGN_W, DESIGN_H, "game of life");
  SetTargetFPS(60);
  SetRandomSeed(10101);

  bool states[2][DIM_W][DIM_H]{};
  auto* curr = &states[0][0];
  auto* next = &states[1][0];

  RenderTexture2D background = LoadRenderTexture(DESIGN_W, DESIGN_H);
  BeginTextureMode(background);
  ClearBackground(RAYWHITE);
  for (int x = 0; x <= DIM_W; ++x)
    for (int y = 0; y <= DIM_H; ++y){
      if (x != DIM_W) DrawLine(x * CELL_W, y * CELL_H, (x+1) * CELL_W, y * CELL_H, BLACK);
      if (y != DIM_H) DrawLine(x * CELL_W, y * CELL_H, x * CELL_W, (y+1) * CELL_H, BLACK);
    }
  EndTextureMode();

  const std::vector<std::pair<int, double>> probs{
    {50, 10.},
    {200, 3.},
    {2500, 1.},
  };
  std::function<void()> randomize{[&]{
    int cnt = 0;
      for (int x = 0; x < DIM_W; ++x)
	for (int y = 0; y < DIM_H; ++y)
	  cnt += curr[x][y];
      double p = 0;
      for (int i = 0; i < probs.size(); ++i)
	if (probs[i].first >= cnt){
	  p = probs[i].second / DIM_W / DIM_H;
	  break;
	}
      for (int x = 0; x < DIM_W; ++x)
	for (int y = 0; y < DIM_H; ++y)
	  if (GetRandomValue(0, 1000000-1)/1000000. < p){
	    DrawRectangle(x * CELL_W, y * CELL_H, CELL_W, CELL_H, curr[x][y] ? RED : GREEN);
	    curr[x][y] = !curr[x][y];
	  } else DrawRectangle(x * CELL_W, y * CELL_H, CELL_W, CELL_H, curr[x][y] ? BLUE : WHITE);
  }};

  const auto neighbours = [&](int x, int y){
    int res = 0;
    for (int dx : {-1, 0, 1})
      for (int dy : {-1, 0, 1})
	if (x+dx >= 0 &&
	    y+dy >= 0 &&
	    x+dx < DIM_W &&
	    y+dy < DIM_H &&
	    (dx || dy))
	  res += curr[x+dx][y+dy];
    return res;
  };
  
  std::function<void()> tick{[&]{
    for (int x = 0; x < DIM_W; ++x)
      for (int y = 0; y < DIM_H; ++y){
	auto n = neighbours(x, y);
	if (curr[x][y]) next[x][y] = n >= 2 && n <= 3;
	else next[x][y] = n == 3;
	DrawRectangle(x * CELL_W, y * CELL_H, CELL_W, CELL_H, next[x][y] ? BLUE : WHITE);
      }
    std::swap(curr, next);
  }};

  auto curr_fn = &randomize;
  auto next_fn = &tick;

  while (!WindowShouldClose())
    {
      BeginDrawing();
      (*curr_fn)();
      std::swap(curr_fn, next_fn);
      // DrawTexture(background.texture, 0, 0, WHITE);
      DrawFPS(10, 10);
      EndDrawing();
    }

  CloseWindow();
  
  return 0;
}
