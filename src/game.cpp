#include "game.h"
#include <iostream>
#include <future>
#include <queue>

Game::Game(std::size_t grid_width, std::size_t grid_height): engine(dev()),
random_w(0, static_cast<int>(grid_width - 1)), random_h(0, static_cast<int>(grid_height - 1)), 
random_food(0, 1), random_snake_speed(0.08, 0.12){
  //spawn player snake 
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine), 2000));

  //spawn two ai snakes
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine), 2000));
  snakes.emplace_back(std::make_unique<Snake>(grid_width, grid_height, random_w(engine), random_h(engine), random_snake_speed(engine), 2000));
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

    // Input, Update, Render - the main game loop. Game exits if player is dead or all opponents dead
    controller.HandleInput(running, *(snakes.front().get()));
    if(!Update() || snakes.size() == 1){ break; };
    renderer.Render(snakes, food.get());

    frame_end = SDL_GetTicks();

    // Keep track of how long each loop through the input/update/render cycle takes.
    frame_count++;
    frame_duration = frame_end - frame_start;

    // After every second, update the window title.
    if (frame_end - title_timestamp >= 1000) {
      renderer.UpdateWindowTitle(snakes.front().get()->GetTimeToLive(), frame_count);
      frame_count = 0;
      title_timestamp = frame_end;
    }

    // If the time for this frame is too small (i.e. frame_duration is
    // smaller than the target ms_per_frame), delay the loop to achieve the correct frame rate.
    if (frame_duration < target_frame_duration) { SDL_Delay(target_frame_duration - frame_duration); }
  }
}

void Game::PlaceFood() {
  //spawn Kendall Jenner or sumo wrestler with 50/50 probability
  int type = random_food(engine);
  food.reset();
  if(type == 1){
    food = std::make_unique<KendallJenner>(SDL_Point());
  } else{
    food = std::make_unique<SumoWrestler>(SDL_Point()); 
  }
  
  int x, y;
  while (true) {
    x = random_w(engine);
    y = random_h(engine);
    // Check that the location is not occupied by a snake item before placing food.
    for(const auto& snake_ptr: snakes){
      if(!snake_ptr.get()->SnakeCell(x, y)){
        food.get()->SetLoation(x, y);
        return;
      }
    }
  }
}

bool Game::Update() {
  //each snake gets its thread to update, first snake is player snake, Update for opponent snakes is called in AStar
  std::vector<std::future<bool>> futures(snakes.size());
  futures.at(0) = std::async(&Snake::Update, snakes.at(0).get());
  for(int i = 1; i < snakes.size(); i++){
    futures.at(i) = std::async(&Game::AStar, this, i);
  }

  //if player is dead, return false
  if(!futures.at(0).get()){ return false; };

  //if opponent is dead erase snake
  for(int i = 1; i < snakes.size(); i++){
    if(!futures.at(i).get()){
      snakes.erase(snakes.begin() + i);
      futures.erase(futures.begin() + i);
      i--;
    }
  }

  //update food consumption
  for(int i = 0; i < snakes.size(); i++){
    int new_x = static_cast<int>(snakes.at(i).get()->head_x);
    int new_y = static_cast<int>(snakes.at(i).get()->head_y);
    // Check if there's food over here
    if (food.get()->GetLocation().x == new_x && food.get()->GetLocation().y == new_y) {
      PlaceFood();
      // Grow snake
      snakes.at(i).get()->GrowBody(food.get()->GetWeight());
    }
  }

  return true;
}

bool Game::AStar(int idx_snake){
  int head_x = snakes.at(idx_snake).get()->head_x;
  int head_y = snakes.at(idx_snake).get()->head_y;
  int grid_h = snakes.at(idx_snake).get()->grid_height;
  int grid_w = snakes.at(idx_snake).get()->grid_width;

  //data structure to store position of a cell, its predecessor, visited flag, distances to snake head and manhattan distance to food
  struct Cell_t{ int cur_x; int cur_y; int prev_x; int prev_y; bool visited; int dist_start; int dist_food; };
  std::vector<std::vector<Cell_t>> cells(grid_h, std::vector<Cell_t>(grid_w, Cell_t{-1, -1, -1, -1, false, std::numeric_limits<int>::max(), -1}));
  for(int row = 0; row < grid_h; row++){
    for(int col = 0; col < grid_w; col++){
      cells.at(row).at(col).cur_x = col;
      cells.at(row).at(col).cur_y = row;
      cells.at(row).at(col).dist_food = std::abs(col - food.get()->GetLocation().x) + std::abs(row - food.get()->GetLocation().y);
    }
  }  

  //A* is breadth first search using sorted queue favoring cells with low distance to food.
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
  int row = food.get()->GetLocation().y, col = food.get()->GetLocation().x;
  while(true){
    int row_prev = cells.at(row).at(col).prev_y;
    int col_prev = cells.at(row).at(col).prev_x;
    if(row_prev == head_y && col_prev == head_x){
      if(row > head_y){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kDown;
      }else if(row < head_y){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kUp;
      }else if(col > head_x){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kRight;
      }else if(col < head_x){
        snakes.at(idx_snake).get()->direction = Snake::Direction::kLeft;
      }
      break;
    }
    if(row_prev == -1 || col_prev == -1){ break; }
    row = row_prev;
    col = col_prev;
  }

  return snakes.at(idx_snake).get()->Update();
}

int Game::GetSnakesLeft() const { return snakes.size(); }