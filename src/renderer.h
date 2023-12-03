#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "SDL.h"
#include "snake.h"
#include "kendall_jenner.h"
#include "sumo_wrestler.h"

class Renderer {
 public:
  Renderer(const std::size_t screen_width, const std::size_t screen_height,
           const std::size_t grid_width, const std::size_t grid_height);
  ~Renderer();

  //void Render(Snake const snake, SDL_Point const &food);
  void Render(const std::vector<std::unique_ptr<Snake>>& snakes, const Food* const food);
  void UpdateWindowTitle(int time_to_live, int fps);

 private:
  SDL_Window *sdl_window;
  SDL_Renderer *sdl_renderer;

  const std::size_t screen_width;
  const std::size_t screen_height;
  const std::size_t grid_width;
  const std::size_t grid_height;
};

#endif