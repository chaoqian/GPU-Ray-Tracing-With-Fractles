
#ifndef __SHAPESPHERE_HPP__
#define __SHAPESPHERE_HPP__

#include "math/CS123Algebra.h"

double sphere_intersect(Vector4d const& p, Vector4d const& v, double no_intersection);
Vector4d sphere_get_normal(Vector4d const& p);
Vector2d sphere_texture_coord(Vector4d const& p);

#endif __SHAPESPHERE_HPP__