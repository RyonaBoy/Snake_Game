#ifndef SUMO_WRESTLER_H
#define SUMO_WRESTLER_H

#include "food.h"

class SumoWrestler: public Food{
public:
  SumoWrestler(SDL_Point location): Food(location) {}
  int GetWeight() final override;
};

#endif