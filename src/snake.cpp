#include "snake.h"
#include <cmath>
#include <iostream>

bool Snake::Update() {
  time_to_live--;
  //std::cout << time_to_live << std::endl;
  if(time_to_live <= 0){ return false; }

  SDL_Point prev_cell{static_cast<int>(head_x), static_cast<int>(head_y)};  // We first capture the head's cell before updating.
  UpdateHead();
  SDL_Point current_cell{static_cast<int>(head_x), static_cast<int>(head_y)};  // Capture the head's cell after updating.

  // Update all of the body vector items if the snake head has moved to a new cell.
  if (current_cell.x != prev_cell.x || current_cell.y != prev_cell.y) { UpdateBody(current_cell, prev_cell); }

  return true;
}

void Snake::UpdateHead() {
  float new_head_x = head_x;
  float new_head_y = head_y;
  switch (direction) {
    case Direction::kUp:
      new_head_y -= speed;
      break;
    case Direction::kDown:
      new_head_y += speed;
      break;
    case Direction::kLeft:
      new_head_x -= speed;
      break;
    case Direction::kRight:
      new_head_x += speed;
      break;
  }

  //snake can be blocked by obstacle
#if 0
  for (auto const &item : body) {
    if (new_head_x == item.x && new_head_y == item.y) { return; }
  }
#endif

  // Wrap the Snake around to the beginning if going off of the screen.
  head_x = fmod(new_head_x + grid_width, grid_width);
  head_y = fmod(new_head_y + grid_height, grid_height);
}

void Snake::UpdateBody(SDL_Point &current_head_cell, SDL_Point &prev_head_cell) {
  // Add previous head location to vector
  body.push_back(prev_head_cell);

  if (!growing) {
    // Remove the tail from the vector.
    body.erase(body.begin());
  } else {
    growing = false;
    size++;
  }
}

void Snake::GrowBody(int food_weight) {
  growing = true;
  time_to_live += food_weight*2;
}

// Inefficient method to check if cell is occupied by snake.
bool Snake::SnakeCell(int x, int y) {
  if (x == static_cast<int>(head_x) && y == static_cast<int>(head_y)) {
    return true;
  }
  for (auto const &item : body) {
    if (x == item.x && y == item.y) {
      return true;
    }
  }
  return false;
}

uint32_t Snake::GetTimeToLive() const { return time_to_live; }