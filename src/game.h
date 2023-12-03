#ifndef GAME_H
#define GAME_H

#include <random>
#include "SDL.h"
#include "controller.h"
#include "renderer.h"
#include "snake.h"
#include "kendall_jenner.h"
#include "sumo_wrestler.h"

class Game {
 public:
  Game(std::size_t grid_width, std::size_t grid_height);
  void Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration);
  int GetSnakesLeft() const;

 private:
  //data structure for all snakes including player (first element)
  std::vector<std::unique_ptr<Snake>> snakes;

  //food
  std::unique_ptr<Food> food;

  std::random_device dev;
  std::mt19937 engine;
  std::uniform_int_distribution<int> random_w;
  std::uniform_int_distribution<int> random_h;
  std::uniform_int_distribution<int> random_food;
  std::uniform_real_distribution<float> random_snake_speed;

  void PlaceFood();

  //returns if player is alive
  bool Update();

  //path finding algorithm for ai snakes, returns if opponent snake is alive
  bool AStar(int idx_snake);
};

#endif