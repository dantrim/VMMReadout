#include "coordinates.h"

Coordinates::Coordinates() :
    x_(0.),
    y_(0.),
    z_(0.)
{
}

void Coordinates::set(double x, double y, double z)
{
    x_ = x;
    y_ = y;
    z_ = z;
}
