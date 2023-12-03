#include "food.h"

SDL_Point Food::GetLocation() const {
  return location;
}

void Food::SetLoation(int x, int y){
  location.x = x;
  location.y = y;
}