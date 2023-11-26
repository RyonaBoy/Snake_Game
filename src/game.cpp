#include "game.h"
#include <iostream>
#include <future>
#include <queue>
#include "SDL.h"

Game::Game(std::size_t grid_width, std::size_t grid_height): engine(dev()), 
random_w(0, static_cast<int>(grid_width - 1)), 
random_h(0, static_cast<int>(grid_height - 1)),
random_snake_speed(0.08, 0.12) {
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine)));
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine)));
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine)));
  PlaceFood();
}

void Game::Run(Controller const &controller, Renderer &renderer, std::size_t target_frame_duration) {
  Uint32 title_timestamp = SDL_GetTicks();
  Uint32 frame_start;
  Uint32 frame_end;
  Uint32 frame_duration;
  int frame_count = 0;
  bool running = true;

  while (running) {
    frame_start = SDL_GetTicks();

    // Input, Update, Render - the main game loop.
    controller.HandleInput(running, *(snakes.front().get()));
    Update();
    renderer.Render(snakes, food);

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(score, frame_count);
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to achieve the correct frame rate.
    if (frame_duration < target_frame_duration) {
      SDL_Delay(target_frame_duration - frame_duration);
    }
  }
}

void Game::PlaceFood() {
  int x, y;
  while (true) {
    x = random_w(engine);
    y = random_h(engine);
    // Check that the location is not occupied by a snake item before placing food.
    for(const auto& snake_ptr: snakes){
      if(!snake_ptr.get()->SnakeCell(x, y)){
        food.x = x;
        food.y = y;
        return;
      }
    }
  }
}

void Game::Update() {
  if (!snakes.front().get()->alive) return;
  //each snake gets its thread to update itself, first snake is player snake
  std::vector<std::future<void>> futures(snakes.size());
  futures.at(0) = std::async(&Snake::Update, snakes.at(0).get());
  for(int i = 1; i < snakes.size(); i++){
    futures.at(i) = std::async(&Game::AStar, this, i);
  }
  //update food
  for(int i = 0; i < snakes.size(); i++){
    futures.at(i).wait();
    int new_x = static_cast<int>(snakes.at(i).get()->head_x);
    int new_y = static_cast<int>(snakes.at(i).get()->head_y);
    // Check if there's food over here
    if (food.x == new_x && food.y == new_y) {
      //score++;
      PlaceFood();
      // Grow snake and increase speed.
      snakes.at(i).get()->GrowBody();
      //snakes.at(i).get()->speed += 0.02;
    }
  }
}

void Game::AStar(int idx_snake){
  int head_x = snakes.at(idx_snake).get()->head_x;
  int head_y = snakes.at(idx_snake).get()->head_y;
  int grid_h = snakes.at(idx_snake).get()->grid_height;
  int grid_w = snakes.at(idx_snake).get()->grid_width;

  //data structure to store position of a cell, its predecessor, visited flag, distances to snake head and manhattan distance to food
  using Cell_t = struct{ int cur_x; int cur_y; int prev_x; int prev_y; bool visited; int dist_start; int dist_food; };
  std::vector<std::vector<Cell_t>> cells(grid_h, std::vector<Cell_t>(grid_w, Cell_t{-1, -1, -1, -1, false, std::numeric_limits<int>::max(), -1}));
  for(int row = 0; row < grid_h; row++){
    for(int col = 0; col < grid_w; col++){
      cells.at(row).at(col).cur_x = col;
      cells.at(row).at(col).cur_y = row;
      cells.at(row).at(col).dist_food = std::abs(col - food.x) + std::abs(row - food.y);
    }
  }  

  //visited marks for either visited cell or obstacle
  for(int i = 0; i < snakes.size(); i++){
    for (SDL_Point const &point : snakes.at(i).get()->body) { 
    cells.at(point.y).at(point.x).visited = true;
    }
  }

//TODO turn queue into queue of pointers, make use of std::move
  //bfs with sorted queue
  auto cmp = [](Cell_t a, Cell_t b){ return a.dist_food < b.dist_food; };
  std::priority_queue<Cell_t, std::vector<Cell_t>, decltype(cmp)> q(cmp);
  q.push(cells.at(head_y).at(head_x));
  while(!q.empty()){
    int row = q.top().cur_y;
    int col = q.top().cur_x;
    for(int offset = -1; offset < 2; offset++){
      //loop through vertical neighbors
      if(offset != 0 && row+offset >= 0 && row+offset < grid_h && !cells.at(row+offset).at(col).visited 
      && cells.at(row).at(col).dist_start + 1 < cells.at(row+offset).at(col).dist_start){
        cells.at(row+offset).at(col).dist_start = cells.at(row).at(col).dist_start + 1;
        cells.at(row+offset).at(col).prev_x = col;
        cells.at(row+offset).at(col).prev_y = row;
        q.push(cells.at(row+offset).at(col));
      }
      //loop through horizontal neighbors
      if(offset != 0 && col+offset >= 0 && col+offset < grid_w && !cells.at(row).at(col+offset).visited
      && cells.at(row).at(col).dist_start + 1 < cells.at(row).at(col+offset).dist_start){
        cells.at(row).at(col+offset).dist_start = cells.at(row).at(col).dist_start + 1;
        cells.at(row).at(col+offset).prev_x = col;
        cells.at(row).at(col+offset).prev_y = row;
        q.push(cells.at(row).at(col+offset));
      }
    }
    q.pop();
    cells.at(row).at(col).visited = true;
  }

  //reconstruct A* path and change direction if necessary
  int row = food.y, col = food.x;
  while(true){
    int row_prev = cells.at(row).at(col).prev_y;
    int col_prev = cells.at(row).at(col).prev_x;
    if(row_prev == head_y && col_prev == head_x){
      if(row > head_y/* && (snakes.at(idx_snake).get()->size == 1 || snakes.at(idx_snake).get()->direction != Snake::Direction::kUp)*/){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kDown;
      }else if(row < head_y/* && (snakes.at(idx_snake).get()->size == 1 || snakes.at(idx_snake).get()->direction != Snake::Direction::kDown)*/){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kUp;
      }else if(col > head_x/* && (snakes.at(idx_snake).get()->size == 1 || snakes.at(idx_snake).get()->direction != Snake::Direction::kLeft)*/){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kRight;
      }else if(col < head_x/* && (snakes.at(idx_snake).get()->size == 1 || snakes.at(idx_snake).get()->direction != Snake::Direction::kRight)*/){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kLeft;
      }
      break;
    }
    if(row_prev == -1 || col_prev == -1){ break; }
    row = row_prev;
    col = col_prev;
  }

  snakes.at(idx_snake).get()->Update();
}

int Game::GetScore() const { return score; }
int Game::GetSize() const { return snakes.front()->size; }