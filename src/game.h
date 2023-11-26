#ifndef GAME_H
#define GAME_H

#include <random>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"

class Game {
 public:
  Game(std::size_t grid_width, std::size_t grid_height);
  void Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration);
  int GetScore() const;
  int GetSize() const;

 private:
  //data structure for all snakes including player (first element)
  std::vector<std::unique_ptr<Snake>> snakes;

  SDL_Point food;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
  std::uniform_real_distribution<float> random_snake_speed;

  int score{0};

  void PlaceFood();
  void Update();
  void AStar(int idx_snake);
};

#endif