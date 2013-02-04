
#ifndef __RAYTRACEGPU_HPP__
#define __RAYTRACEGPU_HPP__

#include <CL/cl.h>
#include <QGLWidget>
#include <QVector>

#include "Scene.hpp"
#include "camera/Camera.h"

class RayTraceGPU
{
public:
    typedef struct ScenePrimitive
    {
        Matrix4x4f T;
        
        //material
        Vector4f cAmbient;
        Vector4f cDiffuse;
        Vector4f cSpecular;
        Vector4f cReflective;
        Vector4f cTransparent;
        cl_float shininess;
        
        cl_int shape;

        //textures
        cl_int texture_id;
        cl_float blend;
        Vector2f repeat;

        cl_float2 pad1;

        //julia
        Vector4f mu;

    } ScenePrimitive;

    RayTraceGPU();
    ~RayTraceGPU();

    bool init(QGLWidget * glwidget);

    inline QString const& get_error_log(void) const { return error_log; }

    inline void setScene(Scene const* scene) {m_scene = scene; m_scene_changed = true;}
    void render(Camera const& camera, GLuint texture_id);

    void dump_info(void);

    static QVector<cl_platform_id> get_platform_list(void);
    static QVector<cl_device_id> get_device_list(cl_platform_id platform, cl_device_type type);

    inline QString const& get_description(void) const { return gpu_description; }
    static QString get_platform_name(cl_platform_id platform);
    static QString RayTraceGPU::get_device_name(cl_device_id device);

    bool get_texture(QString const& filename, GLuint & texture_id);
    bool load_texture(QString const& filename);

    void setJuliaMuX(float value) {m_fractal_mu.x = value; m_scene_changed = true;}
    void setJuliaMuY(float value) {m_fractal_mu.y = value; m_scene_changed = true;}
    void setJuliaMuZ(float value) {m_fractal_mu.z = value; m_scene_changed = true;}
    void setJuliaMuW(float value) {m_fractal_mu.w = value; m_scene_changed = true;}

private:
    cl_context context;
    cl_command_queue queue;
    cl_program program; 
    cl_kernel kernel_image2d;
    QString gpu_description;
    QString error_log;

    //scene
    Scene const* m_scene;
    bool m_scene_changed;
    cl_mem m_lights_dev;
    cl_int m_light_count;
    cl_mem m_primitives_dev; //GPU scene
    cl_int m_primitive_count;

    QMap<QString,GLuint> m_texture;
    Vector4f m_fractal_mu;
};

#endif //__RAYTRACEGPU_HPP__