
#include "ShapeSphere.hpp"

double sphere_intersect(Vector4d const& p, Vector4d const& v, double no_intersection)
{
    static const double r = 0.5;

    assert(v.w==0.0);

    Vector4d pn = p/p.w;
    double a = SQ(v.x)+SQ(v.y)+SQ(v.z);
    double b = 2.0*(pn.x*v.x+pn.y*v.y+pn.z*v.z);
    double c = SQ(pn.x)+SQ(pn.y)+SQ(pn.z)-SQ(r);

    double delta = SQ(b)-4.0*a*c;
    if (delta>0.0)
    {
        delta = std::sqrt(delta);
        double lambda1 = (-b+delta)/(a+a);
        double lambda2 = (-b-delta)/(a+a);

        if (lambda1<0.0)
        {
            lambda1 = no_intersection;
        }
        if (lambda2<0.0)
        {
            lambda2 = no_intersection;
        }
        return std::min(lambda1, lambda2);
    }
    
    return no_intersection;
}

Vector4d sphere_get_normal(Vector4d const& p)
{
    return Vector4d(p.x/p.w, p.y/p.w, p.z/p.w, 0.0).getNormalized();
}

Vector2d sphere_texture_coord(Vector4d const& p)
{
    Vector4d pn = p/p.w;

    //top
    if (EQ(pn.y, 0.5))
    {
        return Vector2d(pn.x+0.5, pn.z+0.5);
    }

    //bottom
    if (EQ(pn.y,-0.5))
    {
        return Vector2d(pn.x+0.5, 0.5-pn.z);
    }

    double phi = std::asin(pn.y/0.5);

    double theta = std::atan2(pn.z, pn.x);
    if (theta<0.0)
    {
        return Vector2d(-theta/(2.0*M_PI), 0.5-phi/M_PI);
    }

    return Vector2d(1.0-theta/(2.0*M_PI), 0.5-phi/M_PI);
}