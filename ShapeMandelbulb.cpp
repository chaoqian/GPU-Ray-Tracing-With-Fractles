
#include "ShapeMandelbulb.hpp"

/*
inline double sphere_de(Vector3d const& p)
{
    const double R = 0.5;
    return p.getMagnitude()-R; // solid sphere, negative interior
}
*/

inline double mandelbulb_de(Vector3d const& pos)
{
	Vector3d z = pos;
	double dr = 1.0;
	double r = 0.0;
    int Iterations = 10;
    double Bailout = 10.0;
    double Power = 8.0;
	for (int i = 0; i < Iterations ; i++) 
    {
        r = z.getMagnitude();
		if (r>Bailout)
        {
            break;
        }
		
		// convert to polar coordinates
		double theta = acos(z.z/r);
		double phi = atan2(z.y,z.x);
		dr =  pow( r, Power-1.0)*Power*dr + 1.0;
		
		// scale and rotate the point
		double zr = pow( r,Power);
		theta = theta*Power;
		phi = phi*Power;
		
		// convert back to cartesian coordinates
		z = zr*Vector3d(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
		z+=pos;
	}
	return 0.5*log(r)*r/dr;
}

double mandelbulb_intersect(Vector4d const& p, Vector4d const& v, double no_intersection, Vector4d & n, double & value)
{
    double totalDistance = 0.0;
    double MaximumRaySteps = 40;
    double MinimumDistance = 0.01;
    double lastDistance = no_intersection;
	int steps;
    Vector3d pos(p.x/p.w, p.y/p.w, p.z/p.w);
    Vector3d dir(v.x, v.y, v.z);
    dir.normalize();
	for (steps=0; steps < MaximumRaySteps; steps++) 
    {
        Vector3d q = pos + totalDistance * dir;
		float distance = mandelbulb_de(q);
		totalDistance += distance;
        if (totalDistance >= no_intersection || distance>=lastDistance)
        {   //no intersection
            break;
        }
		if (distance < MinimumDistance)
        {   //intersection
            q = pos + totalDistance * dir;
            q = q - _EPSILON_*dir;
            Vector3d xDir(MinimumDistance, 0.0, 0.0);
            Vector3d yDir(0.0, MinimumDistance, 0.0);
            Vector3d zDir(0.0, 0.0, 0.0+MinimumDistance);
            n = Vector4d(mandelbulb_de(q+xDir)-mandelbulb_de(q-xDir), mandelbulb_de(q+yDir)-mandelbulb_de(q-yDir), mandelbulb_de(q+zDir)-mandelbulb_de(q-zDir), 0.0).getNormalized();
            value = (1.0-steps)/MaximumRaySteps;
            return totalDistance/v.getMagnitude();
        }
        lastDistance = distance;
	}

    return no_intersection;
}