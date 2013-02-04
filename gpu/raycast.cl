//
// vim:filetype=opencl sw=4 ts=4 expandtab nospell
//

#define EPSILON (100.0f*FLT_EPSILON)
#define MAX_LAMBDA 1e10
#define SQ(x) (x)*(x)
#define EQ(a, b) (fabs((a) - (b)) < EPSILON)
#define matMult4(A, v) (float4)(dot(A.s0123,v), dot(A.s4567,v), dot(A.s89ab,v), dot(A.scdef,v))
#define matTranspose4(A) A.s048c159d26ae37bf
#ifndef GL_INVALID_VALUE
#   define GL_INVALID_VALUE 0x0501
#endif //GL_INVALID_VALUE
#define ITERATIONS_JULIA 10
#define ITERATIONS_MANDELBULB 5
#define EPSILON_JULIA 0.003f
#define DELTA 1e-5f //delta for finite differences

enum PrimitiveType {PRIMITIVE_NONE=0, PRIMITIVE_CUBE=1, PRIMITIVE_CONE=2, PRIMITIVE_CYLINDER=3, PRIMITIVE_SPHERE=4,
                    PRIMITIVE_JULIA=5, PRIMITIVE_MANDELBULB=6, PRIMITIVE_MANDELBOX=7};

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;

typedef struct LightData
{
    float4 color;
    float4 attenuation;
    float4 location;
    float4 spot;

} LightData;

typedef struct ScenePrimitive
{
    float16 T;

    //material
    float4 cAmbient;
    float4 cDiffuse;
    float4 cSpecular;
    float4 cReflective;
    float4 cTransparent;
    float  shininess;

    int shape;

    //textures
    int texture_id;
    float blend;
    float2 repeat;

    float2 pad1;
    
    //fractal parameters
    float4 mu;

} ScenePrimitive;

__private inline float matDeterminant4(__private float16 M) 
{
    float a=M.s0, b=M.s1, c=M.s2, d=M.s3; 
    float e=M.s4, f=M.s5, g=M.s6, h=M.s7; 
    float i=M.s8, j=M.s9, k=M.sa, l=M.sb; 
    float m=M.sc, n=M.sd, o=M.se, p=M.sf; 
    return a*f*k*p - a*f*l*o - a*g*j*p + a*g*l*n + a*h*j*o -
           a*h*k*n - b*e*k*p + b*e*l*o + b*g*i*p - b*g*l*m -
           b*h*i*o + b*h*k*m + c*e*j*p - c*e*l*n - c*f*i*p +
           c*f*l*m + c*h*i*n - c*h*j*m - d*e*j*o + d*e*k*n +
           d*f*i*o - d*f*k*m - d*g*i*n + d*g*j*m;
}

__private inline float16 matInverse4(__private float16 M) 
{
    float a=M.s0, b=M.s1, c=M.s2, d=M.s3; 
    float e=M.s4, f=M.s5, g=M.s6, h=M.s7; 
    float i=M.s8, j=M.s9, k=M.sa, l=M.sb; 
    float m=M.sc, n=M.sd, o=M.se, p=M.sf; 
    float det = matDeterminant4(M);
    return (float16)(
        (f*k*p+g*l*n+h*j*o-f*l*o-g*j*p-h*k*n)/det,
        (b*l*o+c*j*p+d*k*n-b*k*p-c*l*n-d*j*o)/det,
        (b*g*p+c*h*n+d*f*o-b*h*o-c*f*p-d*g*n)/det,
        (b*h*k+c*f*l+d*g*j-b*g*l-c*h*j-d*f*k)/det,
        (e*l*o+h*k*m+g*i*p-e*k*p-g*l*m-h*i*o)/det,
        (a*k*p+c*l*m+d*i*o-a*l*o-c*i*p-d*k*m)/det,
        (a*h*o+c*e*p+d*g*m-a*g*p-c*h*m-d*e*o)/det,
        (a*g*l+c*h*i+d*e*k-a*h*k-c*e*l-d*g*i)/det,
        (e*j*p+f*l*m+h*i*n-e*l*n-f*i*p-h*j*m)/det,
        (a*l*n+b*i*p+d*j*m-a*j*p-b*l*m-d*i*n)/det,
        (a*f*p+b*h*m+d*e*n-a*h*n-b*e*p-d*f*m)/det,
        (a*h*j+b*e*l+d*f*i-a*f*l-b*h*i-d*e*j)/det,
        (e*k*n+f*i*o+g*j*m-e*j*o-f*k*m-g*i*n)/det,
        (a*j*o+b*k*m+c*i*n-a*k*n-b*i*o-c*j*m)/det,
        (a*g*n+b*e*o+c*f*m-a*f*o-b*g*m-c*e*n)/det,
        (a*f*k+b*g*i+c*e*j-a*g*j-b*e*k-c*f*i)/det);
}

__private float4 get_reflection(__private float4 v, __private float4 normal)
{
    float4 V = fast_normalize(v);
    float4 N = fast_normalize(normal);
    return fast_normalize(V - 2.0f*dot(V,N)*N);
}

__private float sphere_intersect(__private float4 p, __private float4 v, __private float no_intersection)
{
    float r = 0.5;
    float lambda = no_intersection;

    p /= p.w; 
    float a = dot(v.xyz,v.xyz);
    float b = 2.0f*dot(p.xyz,v.xyz);
    float c = dot(p.xyz,p.xyz)-SQ(r);

    float delta = SQ(b)-4.0*a*c;
    if (delta>0.0f)
    {
        delta = sqrt(delta);
        lambda = min( max((-b+delta)/(a+a), 0.0f), max((-b-delta)/(a+a), 0.0f) );
    }
    
    return (lambda>0.0f ? lambda : no_intersection);
}

__private float4 sphere_get_normal(__private float4 p)
{
    return fast_normalize((float4)(p.xyz/p.w, 0.0f));
}

__private float2 sphere_texture_coord(__private float4 p)
{
    p /= p.w;

    //top
    if (EQ(p.y, 0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f-p.z);
    }

    //bottom
    if (EQ(p.y,-0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f+p.z);
    }

    float phi = asin(p.y/0.5);

    float theta = atan2(p.z, p.x);
    if (theta<0.0f)
    {
        return (float2)(-theta/(2.0f*M_PI), 0.5f+phi/M_PI);
    }

    return (float2)(1.0f-theta/(2.0f*M_PI), 0.5f+phi/M_PI);
}

__private float cube_intersect(__private float4 p, __private float4 v, __private float no_intersection)
{
    float4 front    = (float4)( 0.0f, 0.0f, 1.0f,-0.5f);
    float4 back     = (float4)( 0.0f, 0.0f,-1.0f,-0.5f);
    float4 top      = (float4)( 0.0f, 1.0f, 0.0f,-0.5f);
    float4 bottom   = (float4)( 0.0f,-1.0f, 0.0f,-0.5f);
    float4 left     = (float4)(-1.0f, 0.0f, 0.0f,-0.5f);
    float4 right    = (float4)( 1.0f, 0.0f, 0.0f,-0.5f);

    //plane intersection
    // lambda = -nT*p / nT*v

    float Q, x, y, z;
    float4 q;
    p /= p.w;

    float lambda_front = no_intersection;
    Q = dot(front, v);
    if (Q<0.0f)
    {
        lambda_front = -dot(front, p)/Q;
        q = p + lambda_front*v;
        if (q.x<-0.5f || q.x>0.5f || q.y<-0.5f || q.y>0.5f)
        {
            lambda_front = no_intersection;
        }
    }

    float lambda_back = no_intersection;
    Q = dot(back, v);
    if (Q<0.0f)
    {
        lambda_back = -dot(back, p)/Q;
        q = p + lambda_back*v;
        if (q.x<-0.5f || q.x>0.5f || q.y<-0.5f || q.y>0.5f)
        {
            lambda_back = no_intersection;
        }
    }
    
    float lambda_top = no_intersection;
    Q = dot(top, v);
    if (Q<0.0f)
    {
        lambda_top = -dot(top, p)/Q;
        q = p + lambda_top*v;
        if (q.x<-0.5f || q.x>0.5f || q.z<-0.5f || q.z>0.5f)
        {
            lambda_top = no_intersection;
        }
    }

    float lambda_bottom = no_intersection;
    Q = dot(bottom, v);
    if (Q<0.0f)
    {
        lambda_bottom = -dot(bottom, p)/Q;
        q = p + lambda_bottom*v;
        if (q.x<-0.5f || q.x>0.5f || q.z<-0.5f || q.z>0.5f)
        {
            lambda_bottom = no_intersection;
        }
    }

    float lambda_left = no_intersection;
    Q = dot(left, v);
    if (Q<0.0f)
    {
        lambda_left = -dot(left, p)/Q;
        q = p + lambda_left*v;
        if (q.y<-0.5f || q.y>0.5f || q.z<-0.5f || q.z>0.5f)
        {
            lambda_left = no_intersection;
        }
    }

    float lambda_right = no_intersection;
    Q = dot(right, v);
    if (Q<0.0f)
    {
        lambda_right = -dot(right, p)/Q;
        q = p + lambda_right*v;
        if (q.y<-0.5f || q.y>0.5f || q.z<-0.5f || q.z>0.5f)
        {
            lambda_right = no_intersection;
        }
    }
    
    return min(no_intersection, min(lambda_front, min(lambda_back, min(lambda_top, min(lambda_bottom, min(lambda_left, lambda_right))))));
}

__private float4 cube_get_normal(__private float4 p) 
{
    p /= p.w;
    return convert_float4((fabs(p+0.5f)<EPSILON) - (fabs(p-0.5f)<EPSILON));
}

__private float2 cube_texture_coord(__private float4 p)
{
    p /= p.w;

    //front
    if (EQ(p.z, 0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f+p.y);
    }

    //back
    if (EQ(p.z,-0.5f))
    {
        return (float2)(0.5f-p.x, 0.5f+p.y);
    }

    //right
    if (EQ(p.x, 0.5f))
    {
        return (float2)(0.5f-p.z, 0.5f+p.y);
    }

    //left
    if (EQ(p.x,-0.5f))
    {
        return (float2)(0.5f+p.z, 0.5f+p.y);
    }

    //top
    if (EQ(p.y, 0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f-p.z);
    }

    //bottom
    if (EQ(p.y,-0.5f))
    {
        return (float2)(0.5f+p.x, p.z+0.5f);
    }

    return (float2)(-1.0f,-1.0f);
}

__private float cylinder_intersect(__private float4 p, __private float4 v, __private float no_intersection)
{
    const float  r = 0.5f;
    const float4 top    = (float4)(0.0f, 1.0f, 0.0f,-0.5f);
    const float4 bottom = (float4)(0.0f,-1.0f, 0.0f,-0.5f);

    float lambda1 = no_intersection;
    float lambda2 = no_intersection;
    float lambda_top = no_intersection;
    float lambda_bottom = no_intersection;

    float Q, x, y1, y2, z;

    p /= p.w;

    float a = SQ(v.x)+SQ(v.z);
    float b = 2.0f*(p.x*v.x+p.z*v.z);
    float c = SQ(p.x)+SQ(p.z)-SQ(r);

    float delta = SQ(b)-4.0f*a*c;
    if (delta>0.0f)
    {
        delta = sqrt(delta);
        lambda1 = (-b+delta)/(a+a);
        y1 = p.y + lambda1*v.y;
        if (lambda1<0.0f || y1<-0.5f || y1>0.5f)
        {
            lambda1 = no_intersection;
        }

        lambda2 = (-b-delta)/(a+a);
        y2 = p.y + lambda2*v.y;
        if (lambda2<0.0f || y2<-0.5f || y2>0.5f)
        {
            lambda2 = no_intersection;
        }
    }

    Q = dot(top, v);
    if (Q<0.0f)
    {
        lambda_top = dot(-top, p)/Q;
        x = p.x + lambda_top*v.x;
        z = p.z + lambda_top*v.z;
        if (SQ(x)+SQ(z)>SQ(0.5f))
        {
            lambda_top = no_intersection;
        }
    }

    Q = dot(bottom, v);
    if (Q<0.0f)
    {
        lambda_bottom = dot(-bottom, p)/Q;
        x = p.x + lambda_bottom*v.x;
        z = p.z + lambda_bottom*v.z;
        if (SQ(x)+SQ(z)>SQ(0.5f))
        {
            lambda_bottom = no_intersection;
        }
    }

    return min(no_intersection, min(lambda1, min(lambda2, min(lambda_top, lambda_bottom))));
}

__private float4 cylinder_get_normal(__private float4 p)
{
    p /= p.w;
    float4 py = (float4)(0.0f, p.y, 0.0f, 0.0f);
    return convert_float4((fabs(py+0.5f)<EPSILON) - (fabs(py-0.5f)<EPSILON)) + ((float)(fabs(p.y)<(0.5f-EPSILON)))*fast_normalize((float4)(p.x, 0.0f, p.z, 0.0f));
}

__private float2 cylinder_texture_coord(__private float4 p)
{
    p /= p.w;

    //top
    if (EQ(p.y, 0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f-p.z);
    }

    //bottom
    if (EQ(p.y,-0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f+p.z);
    }

    float theta = atan2(p.z, p.x);
    if (theta<0.0f)
    {
        return (float2)(-theta/(2.0f*M_PI), 0.5f+p.y);
    }

    return (float2)(1.0f-theta/(2.0f*M_PI), 0.5f+p.y);
}

__private float cone_intersect(__private float4 p, __private float4 v, __private float no_intersection)
{
    const float r = 0.5f;
    const float4 bottom = (float4)(0.0f,-1.0f, 0.0f,-0.5f);

    float lambda1 = no_intersection;
    float lambda2 = no_intersection;
    float lambda_bottom = no_intersection;

    float Q, x, y1, y2, z;

    p /= p.w;

    float a = SQ(v.x)+SQ(v.z)-SQ(v.y*r);
    float b = 2.0f*(p.x*v.x+p.z*v.z+(r-p.y)*v.y*SQ(r));
    float c = SQ(p.x)+SQ(p.z)+(2.0f*r*p.y-SQ(r)-SQ(p.y))*SQ(r);

    float delta = SQ(b)-4.0f*a*c;
    if (delta>0.0f)
    {
        delta = sqrt(delta);
        lambda1 = (-b+delta)/(a+a);
        y1 = p.y + lambda1*v.y;
        if (lambda1<0.0f || y1<-0.5f || y1>0.5f)
        {
            lambda1 = no_intersection;
        }

        lambda2 = (-b-delta)/(a+a);
        y2 = p.y + lambda2*v.y;
        if (lambda2<0.0f || y2<-0.5f || y2>0.5f)
        {
            lambda2 = no_intersection;
        }
    }

    Q = dot(bottom, v);
    if (Q<0.0f)
    {
        lambda_bottom = dot(-bottom, p)/Q;
        x = p.x + lambda_bottom*v.x;
        z = p.z + lambda_bottom*v.z;
        if (SQ(x)+SQ(z)>SQ(0.5f))
        {
            lambda_bottom = no_intersection;
        }
    }

    return min(no_intersection, min(lambda_bottom, min(lambda1, lambda2)));
}

__private float4 cone_get_normal(__private float4 p)
{
    const float r = 0.5f;
    p /= p.w;
    return (float4)(0.0f, -1.0f*(float)(fabs(p.y+0.5f)<EPSILON), 0.0f, 0.0f) + ((float)(fabs(p.y)<(0.5f-EPSILON)))*fast_normalize((float4)(p.x, (r-p.y)*SQ(r), p.z, 0.0f));
}

__private float2 cone_texture_coord(__private float4 p)
{
    p /= p.w;

    //top
    if (EQ(p.y, 0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f-p.z);
    }

    //bottom
    if (EQ(p.y,-0.5f))
    {
        return (float2)(0.5f+p.x, 0.5f+p.z);
    }

    float theta = atan2(p.z, p.x);
    if (theta<0.0f)
    {
        return (float2)(-theta/(2.0f*M_PI), 0.5f+p.y);
    }

    return (float2)(1.0f-theta/(2.0f*M_PI), 0.5f+p.y);
}

__private float terrain_f(__private float3 p)
{
    return (p.y-4.0f*sin(p.x*10.0f*M_PI)*sin(p.z*10.0f*M_PI));
}

__private float terrain_intersect(__private float4 p, __private float4 v, __private float no_intersection)
{
    p /= p.w;
    float3 q = p.xyz;
    float3 dir = fast_normalize(v.xyz);
    float rd = 0.0f;
    float dist = EPSILON_JULIA+EPSILON;
    while (dist>=EPSILON_JULIA && rd<no_intersection)
    {
        dist = terrain_f(q);
        q += dir*dist;
        rd += dist;
    }

    if (dist<EPSILON_JULIA)
    {
        return fast_distance(p.xyz, q.xyz)/fast_length(v);
    }

    return no_intersection;
}

__private float4 terrain_get_normal(__private float4 p)
{
    const float r = 0.5f;
    p /= p.w;
    return fast_normalize((float4)
                (terrain_f((float3)(p.x-DELTA,p.y,p.z)) - terrain_f((float3)(p.x+DELTA,p.y,p.z)), 
                 2.0*DELTA, 
                 terrain_f((float3)(p.x,p.y,p.z-DELTA)) - terrain_f((float3)(p.x,p.y,p.z+DELTA)), 
                 0.0));
}

__private inline float mandelbox_de(__private float3 pos, __private float4 mu)
{
    // precomputed somewhere
    int iters = 8;
    float SCALE = mu.x; //2.0f;
    float MR2 = mu.y; //0.2f;
    float4 scalevec = (float4)(SCALE, SCALE, SCALE, fabs(SCALE)) / MR2;
    const float C1 = fabs(SCALE-1.0f);
    float C2 = pow(fabs(SCALE), (float)(1-iters));

    // distance estimate
    float4 p  = (float4)(pos, 1.0f);
    float4 p0 = p;  // p.w is knighty's DEfactor
    for (int i=0; i<iters; i++)
    {
        p.xyz = clamp(p.xyz, -1.0f, 1.0f) * 2.0f - p.xyz;  // box fold: min3, max3, mad3
        float r2 = dot(p.xyz, p.xyz);  // dp3
        p.xyzw *= clamp(max(MR2/r2, MR2), 0.0f, 1.0f);  // sphere fold: div1, max1.sat, mul4
        p.xyzw = p*scalevec + p0;  // mad4
    }
    return (fast_length(p.xyz) - C1) / p.w - C2;
}

__private float mandelbox_intersect(__private float4 p, __private float4 v, __private float no_intersection, float4 mu)
{
    p /= p.w;
    float3 q = p.xyz;
    float rd = 0.0f;
    float epsilon = mu.z; //0.003f
    float dist = epsilon+EPSILON;
    while (dist>=epsilon && rd<no_intersection)
    {
        dist = mandelbox_de(q, mu);
        q += dist*v.xyz;
        rd += dist;
    }
    if (dist<epsilon)
    {
        return fast_distance(p.xyz, q.xyz)/fast_length(v);
    }
    return no_intersection;
}

__private float4 mandelbox_get_normal(__private float4 p, float4 mu)
{
    float3 qp = (float3)(p.x/p.w, p.y/p.w, p.z/p.w);
    float gx1 = mandelbox_de(qp - (float3)(DELTA, 0.0f, 0.0f), mu);
    float gx2 = mandelbox_de(qp + (float3)(DELTA, 0.0f, 0.0f), mu);
    float gy1 = mandelbox_de(qp - (float3)(0.0f, DELTA, 0.0f), mu);
    float gy2 = mandelbox_de(qp + (float3)(0.0f, DELTA, 0.0f), mu);
    float gz1 = mandelbox_de(qp - (float3)(0.0f, 0.0f, DELTA), mu);
    float gz2 = mandelbox_de(qp + (float3)(0.0f, 0.0f, DELTA), mu);
    return fast_normalize((float4)(gx2-gx1, gy2-gy1, gz2-gz1, 0.0f));
}

__private inline float mandelbulb_de(__private float3 pos)
{
    float3 w = pos;
    float dr = 1.0f;
    float r = 0.0f;
    float bailout = 5.0f;
    for (int i=0; i<ITERATIONS_MANDELBULB; i++)
    {
        r = fast_length(w);
        if (r>bailout)
        {
            break;
        }

        dr = pow(r, 7.0f)*8.0f*dr + 1.0f;
        
        float x = w.x; float x2 = x*x; float x4 = x2*x2;
        float y = w.y; float y2 = y*y; float y4 = y2*y2;
        float z = w.z; float z2 = z*z; float z4 = z2*z2;

        float k3 = x2 + z2;
        float k2 = rsqrt( k3*k3*k3*k3*k3*k3*k3 );
        float k1 = x4 + y4 + z4 - 6.0f*y2*z2 - 6.0f*x2*y2 + 2.0f*z2*x2;
        float k4 = x2 - y2 + z2;

        w.x =  64.0f*x*y*z*(x2-z2)*k4*(x4-6.0f*x2*z2+z4)*k1*k2;
        w.y = -16.0f*y2*k3*k4*k4 + k1*k1;
        w.z = -8.0f*y*k4*(x4*x4 - 28.0f*x4*x2*z2 + 70.0f*x4*z4 - 28.0f*x2*z2*z4 + z4*z4)*k1*k2;
        
        w+=pos;
    }
    return 0.5f*log(r)*r/dr;
}

__private float mandelbulb_intersect(__private float4 p, __private float4 v, __private float no_intersection, float4 mu)
{
    p /= p.w;
    float3 q = p.xyz;
    float rd = 0.0f;
    float epsilon = mu.x; //0.003f
    float dist = epsilon+EPSILON;
    while (dist>epsilon && rd<no_intersection)
    {
        dist = mandelbulb_de(q);
        q += dist*v.xyz;
        rd += dist;
    }
    if (dist<epsilon)
    {
        return fast_distance(p.xyz, q.xyz)/fast_length(v);
    }
    return no_intersection;
}

__private float4 mandelbulb_get_normal(__private float4 p)
{
    float3 qp = (float3)(p.x/p.w, p.y/p.w, p.z/p.w);
    float gx1 = mandelbulb_de(qp - (float3)(DELTA, 0.0f, 0.0f));
    float gx2 = mandelbulb_de(qp + (float3)(DELTA, 0.0f, 0.0f));
    float gy1 = mandelbulb_de(qp - (float3)(0.0f, DELTA, 0.0f));
    float gy2 = mandelbulb_de(qp + (float3)(0.0f, DELTA, 0.0f));
    float gz1 = mandelbulb_de(qp - (float3)(0.0f, 0.0f, DELTA));
    float gz2 = mandelbulb_de(qp + (float3)(0.0f, 0.0f, DELTA));
    return fast_normalize((float4)(gx2-gx1, gy2-gy1, gz2-gz1, 0.0f));
}

///----------------- Julia ------------------------------------------------------------------
// based on: http://users.cms.caltech.edu/~keenan/project_qjulia.html

__private float4 quaternion_mult(__private float4 q1, __private float4 q2)
{
    float4 r;
    float3 t;
    float3 q1yzw = (float3)(q1.y, q1.z, q1.w);
    float3 q2yzw = (float3)(q2.y, q2.z, q2.w);
    float3 c = cross(q1yzw, q2yzw);

    t = q2yzw * q1.x + q1yzw * q2.x + c;
    r.x = q1.x*q2.x - dot(q1yzw, q2yzw);
    r.yzw = t.xyz;

    return r;
}

__private float4 quaternion_sqr(__private float4 q)
{
    float4 r;
    float3 t;
    float3 qyzw = (float3)(q.y, q.z, q.w);

    t     = 2.0f * q.x * qyzw;
    r.x   = q.x*q.x - dot(qyzw, qyzw);
    r.yzw = t.xyz;

    return r;
}

__private float julia_intersect(__private float4 p, __private float4 v, __private float no_intersection, __private float4 mu)
{
    p /= p.w;
    float3 q = p.xyz;
    float rd = 0.0f;
    float dist = EPSILON_JULIA + EPSILON;
    while (dist>=EPSILON_JULIA && rd<no_intersection)
    {
        float4 z = (float4)(q.x, q.y, q.z, 0.0f);
        float4 zp = (float4)(1.0f, 0.0f, 0.0f, 0.0f);
        float zd = 0.0f;
        uint count = 0;
        while(zd<no_intersection && count<ITERATIONS_JULIA)
        {
            zp = 2.0f * quaternion_mult(z, zp);
            z = quaternion_sqr(z) + mu;
            zd = dot(z, z);
            count++;
        }

        float normZ = fast_length(z);
        dist = 0.5f*normZ * half_log(normZ)/fast_length(zp);
        q += v.xyz*dist;
        rd = dot(q, q);
    }

    if (dist<EPSILON_JULIA)
    {
        return fast_distance(p.xyz, q.xyz)/fast_length(v);
    }
    return no_intersection;
}

__private float4 julia_get_normal(__private float4 p, __private float4 mu)
{
    float4 qp = (float4)(p.x/p.w, p.y/p.w, p.z/p.w, 0.0f );
    float4 gx1 = qp - (float4)( DELTA, 0.0f, 0.0f, 0.0f );
    float4 gx2 = qp + (float4)( DELTA, 0.0f, 0.0f, 0.0f );
    float4 gy1 = qp - (float4)( 0.0f, DELTA, 0.0f, 0.0f );
    float4 gy2 = qp + (float4)( 0.0f, DELTA, 0.0f, 0.0f );
    float4 gz1 = qp - (float4)( 0.0f, 0.0f, DELTA, 0.0f );
    float4 gz2 = qp + (float4)( 0.0f, 0.0f, DELTA, 0.0f );

    for (int i=0; i<ITERATIONS_JULIA; i++)
    {
        gx1 = quaternion_sqr(gx1) + mu;
        gx2 = quaternion_sqr(gx2) + mu;
        gy1 = quaternion_sqr(gy1) + mu;
        gy2 = quaternion_sqr(gy2) + mu;
        gz1 = quaternion_sqr(gz1) + mu;
        gz2 = quaternion_sqr(gz2) + mu;
    }

    return fast_normalize((float4)(fast_length(gx2) - fast_length(gx1), 
                                   fast_length(gy2) - fast_length(gy1), 
                                   fast_length(gz2) - fast_length(gz1),
                                   0.0f));
}

///-END------------- Julia ------------------------------------------------------------------

__private float intersect(__constant struct ScenePrimitive * primitives, __private int primitive_count, float4 p, float4 v, float max_lambda,
                             int * index, float16 * Tinv, float4 * n0, float2 * texture_coord)
{
    //intersect with the shapes in the scene
    float lambda = max_lambda;
    for (int i=0; i<primitive_count; i++)
    {
        float16 Tinv0 = matInverse4(primitives[i].T);
        float4 p0 = matMult4(Tinv0, p);
        float4 v0 = matMult4(Tinv0, v);
        float lambda0 = max_lambda;
        float4 mu = primitives[i].mu;
        switch (primitives[i].shape)
        {
            case PRIMITIVE_CUBE:        lambda0 = cube_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_SPHERE:      lambda0 = sphere_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_CYLINDER:    lambda0 = cylinder_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_CONE:        lambda0 = cone_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_JULIA:       lambda0 = julia_intersect(p0, v0, max_lambda, mu); break;
//            case PRIMITIVE_MANDELBULB:  lambda0 = mandelbulb_intersect(p0, v0, max_lambda, mu); break;
            case PRIMITIVE_MANDELBOX:   lambda0 = mandelbox_intersect(p0, v0, max_lambda, mu); break;
        }
        if (lambda0>EPSILON && lambda0<lambda)
        {   //intersection
            float4 q0 = (p0/p.w) + lambda0*v0;
            switch (primitives[i].shape)
            {
                case PRIMITIVE_CUBE:        (*n0) = cube_get_normal(q0);     (*texture_coord) = cube_texture_coord(q0);     break;
                case PRIMITIVE_SPHERE:      (*n0) = sphere_get_normal(q0);   (*texture_coord) = sphere_texture_coord(q0);   break;
                case PRIMITIVE_CYLINDER:    (*n0) = cylinder_get_normal(q0); (*texture_coord) = cylinder_texture_coord(q0); break;
                case PRIMITIVE_CONE:        (*n0) = cone_get_normal(q0);     (*texture_coord) = cone_texture_coord(q0);     break;
                case PRIMITIVE_JULIA:       (*n0) = julia_get_normal(q0-EPSILON_JULIA*v0, mu); (*texture_coord) = sphere_texture_coord(q0); break; 
                case PRIMITIVE_MANDELBULB:  (*n0) = mandelbulb_get_normal(q0-EPSILON_JULIA*v0); (*texture_coord) = sphere_texture_coord(q0); break;
                case PRIMITIVE_MANDELBOX:   (*n0) = mandelbox_get_normal(q0-EPSILON_JULIA*v0, mu); (*texture_coord) = cube_texture_coord(q0); break;
                
            }
            if (!EQ(dot(*n0, fast_normalize(v0)), 0.0f))
            {
                lambda = lambda0;
                (*Tinv) = Tinv0;
                (*index) = i;
            }
        }
    }
    return lambda;
}

float4 get_intensity(float4 p, float4 v, float4 * pr, float4 * vr, float4 * kr,
                          __private float4 global_data, __constant struct LightData * lights, __private int light_count,
                          __constant struct ScenePrimitive * primitives, __private int primitive_count,
                          __read_only image2d_t texture0, __read_only image2d_t texture1,
                          __read_only image2d_t texture2, __read_only image2d_t texture3)
{
    float ka = global_data.x;
    float kd = global_data.y;
    float ks = global_data.z;
    float kt = global_data.w;

    //intersect with the shapes in the scene
    int index = -1;
    float16 Tinv;
    float4 n0;
    float2 texture_coord;
    float lambda = intersect(primitives, primitive_count, p, v, MAX_LAMBDA, &index, &Tinv, &n0, &texture_coord);

    if (lambda>=MAX_LAMBDA)
    {   //no intersection
        (*pr) = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
        (*vr) = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        (*kr) = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
        return  (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    }

    float4 q = p + lambda*v;
    float4 n = matMult4(matTranspose4(Tinv), n0);
    n.w = 0.0f;
    n = fast_normalize(n);

    float4 cAmbient     = primitives[index].cAmbient;
    float4 cDiffuse     = primitives[index].cDiffuse;
    float4 cSpecular    = primitives[index].cSpecular;
    float4 cReflective  = primitives[index].cReflective;
    float4 cTransparent = primitives[index].cTransparent;
    float  shininess    = primitives[index].shininess;

    //texture
    float blend = 0.0f;
    float4 texture_color = (float4)(0.0f, 0.0f, 0.0f, 1.0f);
    if (primitives[index].texture_id!=GL_INVALID_VALUE && primitives[index].texture_id<4 && texture_coord.x>=0.0f)
    {
        blend =  primitives[index].blend;
        texture_coord = texture_coord*primitives[index].repeat - floor(texture_coord*primitives[index].repeat);
        switch (primitives[index].texture_id)
        {
        case 0: texture_color = read_imagef (texture0, sampler, texture_coord); break;
        case 1: texture_color = read_imagef (texture1, sampler, texture_coord); break;
        case 2: texture_color = read_imagef (texture2, sampler, texture_coord); break;
        case 3: texture_color = read_imagef (texture3, sampler, texture_coord); break;
        }
        texture_color.w = 1.0f;
    }
    float blend1 = min(blend, 1.0f), blend0 = 1.0f - blend;


    float4 color = ka*cAmbient;
    
    //foreach light
    for (int i=0; i<light_count; i++)
    {
        float4 light_dir;
        float  light_dist = MAX_LAMBDA;
        float  fatt = 1.0f;
        if (lights[i].location.w!=0.0f)
        {   //point light
            float4 light_pos = lights[i].location/lights[i].location.w;
            light_dist = fast_distance(light_pos.xyz, q.xyz);
            
            //direction
            light_dir = fast_normalize(light_pos-q);
        
            //attenuation
            fatt = min(1.0f/(lights[i].attenuation.x+lights[i].attenuation.y*light_dist+lights[i].attenuation.z*SQ(light_dist)), 1.0f);
        }
        else
        {   //directional light
            light_dir = fast_normalize(-lights[i].location);
        }

        // intersect with the shapes in the scene
        int index1 = -1;
        float16 Tinv1;
        float4 n1;
        float2 texture_coord1;
        float lambda1 = intersect(primitives, primitive_count, q+100.f*EPSILON*n, light_dir, light_dist, &index1, &Tinv1, &n1, &texture_coord1);
        float min_lambda1 = EPSILON;
        switch (primitives[index].shape)
        {
        case PRIMITIVE_JULIA:       min_lambda1 = EPSILON_JULIA; break;
        case PRIMITIVE_MANDELBULB:  min_lambda1 = 10.0f*EPSILON_JULIA; break;
        case PRIMITIVE_MANDELBOX:   min_lambda1 = 10.0f*EPSILON_JULIA; break;
        }
        if (lambda1>min_lambda1 && lambda1<light_dist)
        {   //intersection with other object: skip
            continue;
        }

        //check spot cone
        float penumbra = lights[i].spot.w;
        if (penumbra>0.0f)
        {
            float4 spot_dir = (float4)(lights[i].spot.xyz, 0.0f);
            float cos1 = dot(spot_dir, -light_dir);
            float cos2 = cos(penumbra*M_PI/180.0f);
            if (cos1<cos2)
            {
                continue;
            }
            fatt *= min(1.0f, pow(6.0f*(cos1-cos2)/(1.0f-cos2), 0.6f));
        }


        //diffuse term
        float diffuse = max(dot(n,light_dir), 0.0f);
        color += fatt*lights[i].color*(kd*blend0*cDiffuse+blend1*texture_color)*diffuse;

        //specular term
        float specular = (shininess>1.0f ? pow(max(0.0f, dot(get_reflection(light_dir, n), v)), shininess) : 0.0f);
        color += fatt*lights[i].color*ks*cSpecular*specular;
    }

    (*pr) = q;
    (*vr) = get_reflection(v, n);
    (*kr) = ks*cReflective;

    return color;
}

__kernel void raycast_image2d(__write_only image2d_t image, __private float4 global_data, 
                              __constant struct LightData * lights, __private int light_count,
                              __constant struct ScenePrimitive * primitives, __private int primitive_count,
                              __private float4 p, __private float16 Minv,
                              __read_only image2d_t texture0, __read_only image2d_t texture1,
                              __read_only image2d_t texture2, __read_only image2d_t texture3)
{
    int tx = get_global_id(0); 
    int ty = get_global_id(1);

    float4 pixel = (float4)(0, 0, 0, 1.0f);

    // create a ray in world coordinates
    float4 v = fast_normalize(matMult4(Minv, (float4)(tx, get_global_size(1)-ty, 1.0f, 0.0f)));
    float4 q = p/p.w;
    
    float4 color = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    float4 k = (float4)(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i=0; i<2; i++)
    {
        float4 pr, vr;
        float4 kr;
        color += k*get_intensity(q, v, &pr, &vr, &kr, global_data, lights, light_count, primitives, primitive_count,
                                 texture0, texture1, texture2, texture3);
        q = pr;
        v = vr;
        k = kr;
    }
    
    pixel.xyz = clamp(color.xyz, 0.0f, 1.0f);
    write_imagef (image, (int2)(tx, ty), pixel);
}
