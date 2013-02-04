
#ifndef __SHAPECYLINDER_HPP__
#define __SHAPECYLINDER_HPP__

#include "math/CS123Algebra.h"

double cylinder_intersect(Vector4d const& p, Vector4d const& v, double no_intersection);
Vector4d cylinder_get_normal(Vector4d const& p);
Vector2d cylinder_texture_coord(Vector4d const& p);

#endif __SHAPECYLINDER_HPP__