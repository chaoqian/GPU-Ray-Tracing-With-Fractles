
#include "RayTraceCPU.hpp"

#include <QImage>
#include <QFile>
#include <QFileInfo>

#include "ShapeSphere.hpp"
#include "ShapeCube.hpp"
#include "ShapeCylinder.hpp"
#include "ShapeCone.hpp"
#include "ShapeMandelbulb.hpp"

#define MAX_LAMBDA 1e8

#ifdef _MSC_VER
extern const char * DEFAULT_IMAGE_PATH;
#endif

//test
/*
#define f(x,y,z) (y-4.0*sin(x*10.0*M_PI)*sin(z*10.0*M_PI))
double terrain_intersect(Vector4d const& p, Vector4d const& v, double no_intersection)
{
    Vector3d q = Vector3d(p.x/p.w, p.y/p.w, p.z/p.w);
    Vector3d dir = Vector3d(v.x, v.y, v.z).getNormalized();
    double rd = 0.0;
    double EPSILON_JULIA = 0.03;
    double dist = EPSILON_JULIA+_EPSILON_;
    while (dist>=EPSILON_JULIA && rd<no_intersection)
    {
        dist = f(q.x, q.y, q.z);
        q += dist*dir;
        rd += dist;
    }

    if (dist<EPSILON_JULIA)
    {
        return rd/v.getMagnitude();
    }

    return no_intersection;
}

Vector4d terrain_get_normal(Vector4d const& p)
{
    double eps = 0.005;
    Vector4d n(f(p.x-eps,p.y,p.z) - f(p.x+eps,p.y,p.z), 2.0*eps, f(p.x,p.y,p.z-eps) - f(p.x,p.y,p.z+eps), 0.0);
    return n.getNormalized();
}
#undef f
*/

RayTraceCPU::RayTraceCPU() :
    m_texture()
{
}

RayTraceCPU::~RayTraceCPU()
{
}

void RayTraceCPU::render(Scene const* scene, Camera const& camera, QImage & canvas)
{
    int width = canvas.width();
    int height = canvas.height();

    if (!width || !height)
    {
        return;
    }

    if (!scene)
    {
        return;
    }

    //camera
    Vector2d f(2.0/width, 2.0/height);
    Matrix4x4 K(f.x, 0.0,-1.0, 0.0,
                0.0,-f.y, 1.0, 0.0,
                0.0, 0.0,-1.0, 0.0,
                0.0, 0.0, 0.0, 1.0);
    Matrix4x4 M = camera.getCameraMatrix()*camera.getModelviewMatrix();
    Matrix4x4 Minv = M.getInverse()*K;
    Vector4d p = Minv*Vector4(0.0, 0.0, 0.0, 1.0);
    p /= p.w;

    Vector4d v;

    for (int h=0; h<height; h++)
    {
        QRgb * row = reinterpret_cast<QRgb *>(canvas.scanLine(h));
        for (int w=0; w<width; w++)
        {
            row[w] = qRgb(0, 0, 0);

            //create a ray in world coordinates
            v = Minv*Vector4d(w, h, 1.0, 0.0);
            v.normalize();
            Vector4d q = p;

            Vector4f color = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
            Vector4f k = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
            for (int i=0; i<2; i++)
            {
                Vector4d pr, vr;
                Vector4f kr;
                color += k*get_intensity(scene, q, v, pr, vr, kr);
                q = pr;
                v = vr;
                k = kr;
            }

            row[w] = qRgb((color.x>1.0?255:255*color.x), (color.y>1.0?255:255*color.y), (color.z>1.0?255:255*color.z));
        }
    }
}

bool RayTraceCPU::get_texture(QString const& filename, QImage const** image)
{
    QMap<QString,QImage>::ConstIterator iter = m_texture.constFind(filename);
    if (iter!=m_texture.constEnd())
    {   //found
        if (image)
        {
            *image = &(*iter);
        }
        return !iter->isNull();
    }

    //not found: try to load
    if (load_texture(filename))
    {   //loaded retry
        iter = m_texture.constFind(filename);
        if (iter!=m_texture.constEnd())
        {   //found
            if (image)
            {
                *image = &(*iter);
            }
            return !iter->isNull();
        }
    }

    //failed: add the key anyway
    m_texture.insert(filename, QImage());

    return false;
}

bool RayTraceCPU::load_texture(QString const& filename)
{
    //QMutexLocker locker(&m_lock);
    QMap<QString,QImage>::ConstIterator iter = m_texture.constFind(filename);
    if (iter!=m_texture.constEnd())
    {   //found: not load, no error
        return true;
    }

    QFile file(filename);

    QImage image, texture;
    if (!file.exists() || !image.load(file.fileName()))
    {
#ifdef _MSC_VER
        //Windows, try an alternative path
        QFile winFile(DEFAULT_IMAGE_PATH+QFileInfo(file).fileName());
        if (!winFile.exists() || !image.load(winFile.fileName()))
        {
            std::cerr << "[RayTraceCPU] (win) Image loaded FAILED: " << winFile.fileName().toStdString() << std::endl;
#endif
            std::cerr << "[RayTraceCPU] Image loaded FAILED: " << filename.toStdString() << std::endl;
            return false;
#ifdef _MSC_VER
        }//if
#endif
    }

    std::cerr << "[RayTraceCPU] Image loaded: " << filename.toStdString() << std::endl;
    m_texture.insert(filename, image);
    return true;
}

Vector4f RayTraceCPU::get_intensity(Scene const* scene, Vector4d const& p, Vector4d const& v, Vector4d & pr, Vector4d & vr, Vector4f & kr)
{
    if (EQ(v.getMagnitude2(),0.0))
    {   //null vector
        pr = Vector4d(0.0, 0.0, 0.0, 1.0);
        vr = Vector4d(0.0, 0.0, 0.0, 0.0);
        kr = Vector4f(0.0f,0.0f,0.0f,0.0f);
        return Vector4f(0.0f,0.0f,0.0f,0.0f);
    }

    //lights
    QVector<Scene::LightData> const& lights = scene->getLights();

    //global data
    Vector4f global_data = scene->getGlobal();
    float ka = global_data.x;
    float kd = global_data.y;
    float ks = global_data.z;
    //float kt = global_data.w;

    //shapes
    QList<Scene::ScenePrimitive> const& primitives = scene->getPrimitives();

    //intersect with the shapes in the scene
    int index = -1;
    Matrix4x4 Tinv;
    Vector4d n0;
    Vector2d texture_coord;
    double lambda = intersect(primitives, p, v, MAX_LAMBDA, index, Tinv, n0, texture_coord);

    if (lambda>=MAX_LAMBDA)
    {   //no intersection
        pr = Vector4d(0.0, 0.0, 0.0, 1.0);
        vr = Vector4d(0.0, 0.0, 0.0, 0.0);
        kr = Vector4f(0.0f,0.0f,0.0f,0.0f);
        return Vector4f(0.0f,0.0f,0.0f,0.0f);
    }

    Vector4d q = p + lambda*v;
    Vector4d n = Tinv.getTranspose()*n0;
    n.w = 0.0;
    n.normalize();

    CS123SceneMaterial const& material = primitives.at(index).material;
    Vector4f cAmbient(material.cAmbient.r, material.cAmbient.g, material.cAmbient.b, material.cAmbient.a);
    Vector4f cDiffuse(material.cDiffuse.r, material.cDiffuse.g, material.cDiffuse.b, material.cDiffuse.a);
    Vector4f cSpecular(material.cSpecular.r, material.cSpecular.g, material.cSpecular.b, material.cSpecular.a);
    Vector4f cReflective(material.cReflective.r, material.cReflective.g, material.cReflective.b, material.cReflective.a);
    float shininess = material.shininess;

    //texture
    float blend = 0.f;
    Vector4f texture_color(0.0f, 0.0f, 0.0f, 1.0f);
    QImage const* texture;
    if (texture_coord.x>0.0 && material.textureMap && material.textureMap->isUsed 
        && get_texture(QString::fromStdString(material.textureMap->filename), &texture))
    {
        blend = material.blend;

        QRgb rgb = texture->pixel(static_cast<int>(texture_coord.x*material.textureMap->repeatU*texture->width())%texture->width(), 
                                    static_cast<int>(texture_coord.y*material.textureMap->repeatV*texture->height())%texture->height());
        texture_color.x = qRed(rgb)/255.f;
        texture_color.y = qGreen(rgb)/255.f;
        texture_color.z = qBlue(rgb)/255.f;
        texture_color.w = 1.0f;
    }
    float blend1 = std::min(blend, 1.f), blend0 = 1.f - blend;

    Vector4f color = ka*cAmbient;
    foreach (Scene::LightData const& light, lights)
    {
        Vector4d light_dir;
        float light_dist = MAX_LAMBDA;
        float fatt = 1.0f;
        if (light.location.w!=0.0)
        {   //point light
            Vector4d light_pos(light.location.x/light.location.w, light.location.y/light.location.w, light.location.z/light.location.w, 1.0);
            float light_dist = light_pos.getDistance(q);

            //direction
            light_dir = Vector4d(light_pos-(q/q.w)).getNormalized();

            //attenuation
            fatt = std::min(static_cast<float>(1.0/(light.attenuation.x+light.attenuation.y*light_dist+light.attenuation.z*SQ(light_dist))), 1.f);
        }
        else 
        {   //directional light
            light_dir = -Vector4d(light.location.x, light.location.y, light.location.z, light.location.w).getNormalized();
        }

        // intersect with the shapes in the scene
        /*int index1 = -1;
        Matrix4x4 Tinv1;
        Vector4d n1;
        Vector2d texture_coord1;
        if (intersect(primitives, q+_EPSILON_*n, light_dir, light_dist, index1, Tinv1, n1, texture_coord1)<light_dist)
        {   //intersection with other object: skip
            continue;
        }*/

        //check spot cone
        float penumbra = light.spot.w;
        if (penumbra>0.0f)
        {
            Vector4d spot_dir = Vector4d(light.spot.x, light.spot.y, light.spot.z, 0.0f);
            double cos1 = spot_dir.dot(-light_dir);
            double cos2 = std::cos(penumbra*M_PI/180.0);
            if (cos1<cos2)
            {
                continue;
            }
            fatt *= std::min(1.0, std::pow(6.0*(cos1-cos2)/(1.0-cos2), 0.6));
        }

        //diffuse term
        double diffuse = std::max(n.dot(light_dir), 0.0);
        color += fatt*light.color*(kd*blend0*cDiffuse+blend1*texture_color)*diffuse;

        //specular term
        double specular = (shininess>1.f ? std::pow(std::max(0.f, static_cast<float>(light_dir.getReflection(n).dot(v))), shininess) : 0.f);
        color += fatt*light.color*ks*cSpecular*specular;
    }

    //reflection
    pr = q + n*_EPSILON_;
    vr = v.getReflection(n);
    kr = ks*cReflective;

    return color;
}

float RayTraceCPU::intersect(QList<Scene::ScenePrimitive> const& primitives, Vector4d const& p, Vector4d const& v, float max_lambda,
                             int & index, Matrix4x4 & Tinv, Vector4d & n0, Vector2d & texture_coord)
{
    //intersect with the shapes in the scene
    double lambda = max_lambda;
    for (int i=0; i<primitives.size(); i++)
    {
        Scene::ScenePrimitive const& primitive = primitives.at(i);
        Matrix4x4 Tinv0 = primitive.T.getInverse();
        Vector4d p0 = Tinv0*p;
        Vector4d v0 = Tinv0*v;
        Vector4d mn0;
        double lambda0 = max_lambda;
        switch (primitive.shape)
        {
            case PRIMITIVE_CUBE:        lambda0 = cube_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_SPHERE:      lambda0 = sphere_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_CYLINDER:    lambda0 = cylinder_intersect(p0, v0, max_lambda); break;
            case PRIMITIVE_CONE:        lambda0 = cone_intersect(p0, v0, max_lambda); break;
            //case PRIMITIVE_MANDELBULB:  lambda0 = mandelbulb_intersect(p0, v0, max_lambda, mn0, value); break;
        }
        if (lambda0>_EPSILON_ && lambda0<lambda)
        {   //intersection
            Vector4d q0 = (p0/p.w) + lambda0*v0;
            switch (primitives[i].shape)
            {
                case PRIMITIVE_CUBE:        n0 = cube_get_normal(q0);       texture_coord = cube_texture_coord(q0);     break;
                case PRIMITIVE_SPHERE:      n0 = sphere_get_normal(q0);     texture_coord = sphere_texture_coord(q0);   break;
                case PRIMITIVE_CYLINDER:    n0 = cylinder_get_normal(q0);   texture_coord = cylinder_texture_coord(q0); break;
                case PRIMITIVE_CONE:        n0 = cone_get_normal(q0);       texture_coord = cone_texture_coord(q0);     break;
                //case PRIMITIVE_MANDELBULB:  n0 = mn0; texture_coord = mandelbulb_texture_coord(q0);     break;
            }
            if (!EQ(n0.dot(v0.getNormalized()), 0.0))
            {
                lambda = lambda0;
                Tinv = Tinv0;
                index = i;
            }
        }
    }
    return lambda;
}