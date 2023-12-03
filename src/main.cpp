#include <iostream>
#include "controller.h"
#include "game.h"
#include "renderer.h"

int main() {
  constexpr std::size_t kFramesPerSecond{60};
  constexpr std::size_t kMsPerFrame{1000 / kFramesPerSecond};
  constexpr std::size_t kScreenWidth{640};
  constexpr std::size_t kScreenHeight{640};
  constexpr std::size_t kGridWidth{32};
  constexpr std::size_t kGridHeight{32};

  Renderer renderer(kScreenWidth, kScreenHeight, kGridWidth, kGridHeight);
  std::cout << "renderer created\n";
  Controller controller;
  std::cout << "controller created\n";
  Game game(kGridWidth, kGridHeight);
  std::cout << "game created\n";
  game.Run(controller, renderer, kMsPerFrame);
  if(game.GetSnakesLeft() == 1){ std::cout << "You won!\n"; }
  else{ std::cout << "You lost!\n"; }
  std::cout << "Game has terminated successfully!\n";
  return 0;
}