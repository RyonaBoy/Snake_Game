#ifndef SNAKE_H
#define SNAKE_H

#include <vector>
#include "SDL.h"

class Snake {
 public:
  enum class Direction { kUp, kDown, kLeft, kRight };

  Snake(int grid_width, int grid_height, int spawn_x, int spawn_y, float speed_genetics, uint32_t time): 
  grid_width(grid_width), grid_height(grid_height), head_x(spawn_x), head_y(spawn_y), speed(speed_genetics), time_to_live(time) {}

  //returns if snake is dead
  bool Update();

  void GrowBody(int food_weight);
  bool SnakeCell(int x, int y);
  uint32_t GetTimeToLive() const;

  Direction direction = Direction::kUp;

  const int grid_width;
  const int grid_height;
  float speed;
  int size{1};
  float head_x;
  float head_y;
  std::vector<SDL_Point> body;

 private:
  void UpdateHead();
  void UpdateBody(SDL_Point &current_cell, SDL_Point &prev_cell);

  bool growing{false};
  uint32_t time_to_live = 2000;
};

#endif