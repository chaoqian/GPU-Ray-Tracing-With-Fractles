
#ifndef __SHAPECONE_HPP__
#define __SHAPECONE_HPP__

#include "math/CS123Algebra.h"

double cone_intersect(Vector4d const& p, Vector4d const& v, double no_intersection);
Vector4d cone_get_normal(Vector4d const& p);
Vector2d cone_texture_coord(Vector4d const& p);

#endif __SHAPECONE_HPP__