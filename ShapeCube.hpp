
#ifndef __SHAPECUBE_HPP__
#define __SHAPECUBE_HPP__

#include "math/CS123Algebra.h"

double cube_intersect(Vector4d const& p, Vector4d const& v, double no_intersection);
Vector4d cube_get_normal(Vector4d const& p);
Vector2d cube_texture_coord(Vector4d const& p);

#endif __SHAPECUBE_HPP__