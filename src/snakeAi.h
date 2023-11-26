#ifndef SNAKEAI_H
#define SNAKEAI_H

#include "snake.h"

class SnakeAi: public Snake{
 public:
  SnakeAi(int grid_width, int grid_height): Snake(grid_width, grid_height){}

  void Update() override;
 private:

  void PlanPath();
};

#endif