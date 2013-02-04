
#ifndef __RAYTRACECPU_HPP__
#define __RAYTRACECPU_HPP__

#include <QMap>

#include "Scene.hpp"
#include "camera/Camera.h"

class RayTraceCPU
{
public:
    RayTraceCPU();
    ~RayTraceCPU();

    void render(Scene const* scene, Camera const& camera, QImage & canvas);

private:
    bool get_texture(QString const& filename, QImage const** image);
    bool load_texture(QString const& filename);

    Vector4f get_intensity(Scene const* scene, Vector4d const& p, Vector4d const& v, Vector4d & pr, Vector4d & vr, Vector4f & kr);
    float intersect(QList<Scene::ScenePrimitive> const& primitives, Vector4d const& p, Vector4d const& v, float max_lambda,
                    int & index, Matrix4x4 & Tinv, Vector4d & n0, Vector2d & texture_coord);

    QMap<QString,QImage> m_texture;
};

#endif //__RAYTRACECPU_HPP__