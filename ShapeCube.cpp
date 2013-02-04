
#include "ShapeCube.hpp"

double cube_intersect(Vector4d const& p, Vector4d const& v, double no_intersection)
{
    static const Vector4d front(0.0, 0.0, 1.0,-0.5);
    static const Vector4d back(0.0, 0.0,-1.0,-0.5);
    static const Vector4d top(0.0, 1.0, 0.0,-0.5);
    static const Vector4d bottom(0.0,-1.0, 0.0,-0.5);
    static const Vector4d left(-1.0, 0.0, 0.0,-0.5);
    static const Vector4d right(1.0, 0.0, 0.0,-0.5);

    //plane intersection
    // lambda = -nT*p / nT*v

    double Q, x, y, z;
    Vector4d pn = p/p.w;

    assert(v.w==0.0 && pn.w==1.0);

    double lambda_front = no_intersection;
    Q = front.dot(v);
    if (Q<0.0)
    {
        lambda_front = -front.dot(pn)/Q;
        x = pn.x + lambda_front*v.x;
        y = pn.y + lambda_front*v.y;
        if (x<-0.5 || x>0.5 || y<-0.5 || y>0.5)
        {
            lambda_front = no_intersection;
        }
    }

    double lambda_back = no_intersection;
    Q = back.dot(v);
    if (Q<0.0)
    {
        lambda_back = -back.dot(pn)/Q;
        x = pn.x + lambda_back*v.x;
        y = pn.y + lambda_back*v.y;
        if (x<-0.5 || x>0.5 || y<-0.5 || y>0.5)
        {
            lambda_back = no_intersection;
        }
    }
    
    double lambda_top = no_intersection;
    Q = top.dot(v);
    if (Q<0.0)
    {
        lambda_top = -top.dot(pn)/Q;
        x = pn.x + lambda_top*v.x;
        z = pn.z + lambda_top*v.z;
        if (x<-0.5 || x>0.5 || z<-0.5 || z>0.5)
        {
            lambda_top = no_intersection;
        }
    }

    double lambda_bottom = no_intersection;
    Q = bottom.dot(v);
    if (Q<0.0)
    {
        lambda_bottom = -bottom.dot(pn)/Q;
        x = pn.x + lambda_bottom*v.x;
        z = pn.z + lambda_bottom*v.z;
        if (x<-0.5 || x>0.5 || z<-0.5 || z>0.5)
        {
            lambda_bottom = no_intersection;
        }
    }

    double lambda_left = no_intersection;
    Q = left.dot(v);
    if (Q<0.0)
    {
        lambda_left = -left.dot(pn)/Q;
        y = pn.y + lambda_left*v.y;
        z = pn.z + lambda_left*v.z;
        if (y<-0.5 || y>0.5 || z<-0.5 || z>0.5)
        {
            lambda_left = no_intersection;
        }
    }

    double lambda_right = no_intersection;
    Q = right.dot(v);
    if (Q<0.0)
    {
        lambda_right = -right.dot(pn)/Q;
        y = pn.y + lambda_right*v.y;
        z = pn.z + lambda_right*v.z;
        if (y<-0.5 || y>0.5 || z<-0.5 || z>0.5)
        {
            lambda_right = no_intersection;
        }
    }

    return std::min(no_intersection, std::min(lambda_front, std::min(lambda_back, std::min(lambda_top, std::min(lambda_bottom, std::min(lambda_left, lambda_right))))));
}

Vector4d cube_get_normal(Vector4d const& p) 
{
    Vector4d n(0.0, 0.0, 0.0, 0.0);

    Vector4d pn=p/p.w;
    if (EQ(pn.z,0.5))
    {   //front
        n.z = 1.0;
    }
    else if (EQ(pn.z,-0.5))
    {   //back
        n.z = -1.0;
    }
    else if (EQ(pn.y,0.5))
    {   //top
        n.y = 1.0;
    }
    else if (EQ(pn.y,-0.5))
    {   //bottom
        n.y = -1.0;
    }
    else if (EQ(pn.x,0.5))
    {   //right
        n.x = 1.0;
    }
    else if (EQ(pn.x,-0.5))
    {   //left
        n.x = -1.0;
    }

    return n;
}

Vector2d cube_texture_coord(Vector4d const& p)
{
    Vector4d pn = p/p.w;

    //front
    if (EQ(pn.z, 0.5))
    {
        return Vector2d(pn.x+0.5, 0.5-pn.y);
    }

    //back
    if (EQ(pn.z,-0.5))
    {
        return Vector2d(0.5-pn.x, 0.5-pn.y);
    }

    //right
    if (EQ(pn.x, 0.5))
    {
        return Vector2d(0.5-pn.z, 0.5-pn.y);
    }

    //left
    if (EQ(pn.x,-0.5))
    {
        return Vector2d(pn.z+0.5, 0.5-pn.y);
    }

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

    return Vector2d(-1.0,-1.0);
}