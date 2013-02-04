
#ifndef __SHAPESMANDELBULB_HPP__
#define __SHAPESMANDELBULB_HPP__

#include "math/CS123Algebra.h"

double mandelbulb_intersect(Vector4d const& p, Vector4d const& v, double no_intersection, Vector4d & n, double & value);
inline Vector2d mandelbulb_texture_coord(Vector4d const& p) {return Vector2d(-1.0, -1.0);}

#endif __SHAPESMANDELBULB_HPP__