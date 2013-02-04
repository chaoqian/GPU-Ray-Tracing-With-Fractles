
#include "ShapeCone.hpp"

double cone_intersect(Vector4d const& p, Vector4d const& v, double no_intersection)
{
    static const double r = 0.5;
    static const Vector4d bottom(0.0,-1.0, 0.0,-0.5);

    double lambda1 = no_intersection;
    double lambda2 = no_intersection;
    double lambda_bottom = no_intersection;

    double Q, x, y1, y2, z;

    assert(v.w==0.0);

    Vector4d pn = p/p.w;
    double a = SQ(v.x)+SQ(v.z)-SQ(v.y*r);
    double b = 2.0*(pn.x*v.x+pn.z*v.z+(r-pn.y)*v.y*SQ(r));
    double c = SQ(pn.x)+SQ(pn.z)+(2.0*r*pn.y-SQ(r)-SQ(pn.y))*SQ(r);

    double delta = SQ(b)-4.0*a*c;
    if (delta>0.0)
    {
        delta = std::sqrt(delta);
        lambda1 = (-b+delta)/(a+a);
        y1 = pn.y + lambda1*v.y;
        if (lambda1<0.0 || y1<-0.5 || y1>0.5)
        {
            lambda1 = no_intersection;
        }

        lambda2 = (-b-delta)/(a+a);
        y2 = pn.y + lambda2*v.y;
        if (lambda2<0.0 || y2<-0.5 || y2>0.5)
        {
            lambda2 = no_intersection;
        }
    }

    Q = bottom.dot(v);
    if (Q<0.0)
    {
        lambda_bottom = -bottom.dot(pn)/Q;
        x = pn.x + lambda_bottom*v.x;
        z = pn.z + lambda_bottom*v.z;
        if (SQ(x)+SQ(z)>SQ(0.5))
        {
            lambda_bottom = no_intersection;
        }
    }

    return std::min(no_intersection, std::min(lambda_bottom,std::min(lambda1,lambda2)));
}

Vector4d cone_get_normal(Vector4d const& p)
{
    static const double r = 0.5;
    Vector4d pn = p/p.w;
    if(EQ(pn.y,-0.5))
    {
        return Vector4d(0.0, -1.0, 0.0, 0.0);
    }
    if (p.y>-0.5 && p.y<=0.5)
    {
        return Vector4d(pn.x, (r-pn.y)*SQ(r), pn.z, 0.0).getNormalized();
    }
    return Vector4d();
}

Vector2d cone_texture_coord(Vector4d const& p)
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

    double theta = std::atan2(pn.z, pn.x);
    if (theta<0.0)
    {
        return Vector2d(-theta/(2.0*M_PI), 0.5-pn.y);
    }

    return Vector2d(1.0-theta/(2.0*M_PI), 0.5-pn.y);
}