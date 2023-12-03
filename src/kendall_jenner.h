#ifndef KENDALL_JENNER_H
#define KENDALL_JENNER_H

#include "food.h"

class KendallJenner: public Food{
public:
  KendallJenner(SDL_Point location): Food(location) {}
  int GetWeight() final override;
};

#endif