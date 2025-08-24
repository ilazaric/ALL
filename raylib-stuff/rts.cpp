#include "raylib.h"
#include "raymath.h"

#include <sstream>
#include <string>
#include <cassert>

#define FWD(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

std::string str(auto&&... args){
  std::stringstream ss;
  (ss << ... << FWD(args));
  return std::move(ss).str();
}

struct Drone {
  enum class State { MOVE, MINE, MOVEI, DEPOSIT };
  Vector2 pos;
  State state;
  float rem_time;
};

int main(){
  const int W = 1280, H = 720;
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(W, H, "RTS");
  SetTargetFPS(60);

  size_t money = 100;
  std::vector<Drone> drones;
  const size_t drone_cost = 100;
  const float drone_width = 10;
  const Vector2 mine_pos{200, 100};
  const Vector2 base_pos{W/2., H/2.};
  const float mine_width = 30;
  const float base_width = 70;
  const float drone_speed = 500;
  const float mining_time = 2;
  const float deposit_time = 2;
  const size_t drone_earn = 30;

  while (!WindowShouldClose()){
    const float dt = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE) && money >= drone_cost){
      money -= drone_cost + drone_earn;
      drones.emplace_back();
      auto& drone = drones.back();
      drone.pos = base_pos;
      drone.state = Drone::State::DEPOSIT;
      drone.rem_time = 0;
    }

    for (auto& drone : drones){
      if (drone.state == Drone::State::MOVE){
	const float ldt = std::min(dt, drone.rem_time);
	const float d = sqrtf((mine_pos.x - drone.pos.x) *
			      (mine_pos.x - drone.pos.x) +
			      (mine_pos.y - drone.pos.y) *
			      (mine_pos.y - drone.pos.y));
	drone.pos.x += ldt * (mine_pos.x - drone.pos.x) / d * drone_speed;
	drone.pos.y += ldt * (mine_pos.y - drone.pos.y) / d * drone_speed;
      }
      if (drone.state == Drone::State::MOVEI){
	const float ldt = std::min(dt, drone.rem_time);
	const float d = sqrtf((base_pos.x - drone.pos.x) *
			      (base_pos.x - drone.pos.x) +
			      (base_pos.y - drone.pos.y) *
			      (base_pos.y - drone.pos.y));
	drone.pos.x += ldt * (base_pos.x - drone.pos.x) / d * drone_speed;
	drone.pos.y += ldt * (base_pos.y - drone.pos.y) / d * drone_speed;
      }
      if (drone.rem_time <= dt){
	switch (drone.state){
	case Drone::State::MOVE:
	  drone.state = Drone::State::MINE;
	  drone.rem_time = mining_time;
	  break;
	case Drone::State::MINE:
	  drone.state = Drone::State::MOVEI;
	  drone.rem_time = (Vector2Distance(drone.pos, base_pos) - base_width - drone_width) / drone_speed;
	  break;
	case Drone::State::MOVEI:
	  drone.state = Drone::State::DEPOSIT;
	  drone.rem_time = deposit_time;
	  break;
	case Drone::State::DEPOSIT:
	  money += drone_earn;
	  drone.state = Drone::State::MOVE;
	  drone.rem_time = (Vector2Distance(drone.pos, mine_pos) - mine_width - drone_width) / drone_speed;
	  break;
	default:
	  assert(false);
	}
      } else {
	drone.rem_time -= dt;
      }
    }

    BeginDrawing();
    ClearBackground(WHITE);

    DrawCircleV(base_pos, base_width, GRAY);
    DrawCircleV(mine_pos, mine_width, BROWN);

    for (auto& drone : drones){
      DrawCircleLinesV(drone.pos, drone_width, GREEN);
    }
    
    DrawFPS(10, 10);
    DrawText(str("Cash: ", money).c_str(), 10, 30, 20, ORANGE);
    DrawText(str("Drone cost: ", drone_cost).c_str(), 10, 50, 20, GREEN);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
