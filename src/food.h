#ifndef FOOD_H
#define FOOD_H

#include "SDL.h"

constexpr long double operator"" _t(long double x){ return x*1000.0; }
constexpr long double operator"" _kg(long double x){ return x; };
constexpr long double operator"" _g(long double x){return x*1000.0; }

class Food {

public:
  Food(SDL_Point location): location(location){}
  ~Food(){}
  virtual int GetWeight() = 0;
  SDL_Point GetLocation() const;
  void SetLoation(int x, int y);

protected:
  SDL_Point location;
};

#endif