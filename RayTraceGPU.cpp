
#include "RayTraceGPU.hpp"

#include <CL/cl_gl.h>

#include <QImage>
#include <QFile>
#include <QFileInfo>

#include "math/CS123Algebra.h"

#define CL_ENSURE_SUCCESS(rv,ret,msg) if(rv) {error_log = QString(msg); return ret;}

#ifdef _MSC_VER
extern const char * DEFAULT_IMAGE_PATH;
#endif

RayTraceGPU::RayTraceGPU() :
    context(NULL),
    queue(NULL),
    program(NULL),
    kernel_image2d(NULL),
    gpu_description(),
    error_log(),
    m_scene(NULL),
    m_scene_changed(true),
    m_lights_dev(NULL),
    m_light_count(0),
    m_primitives_dev(NULL),
    m_primitive_count(0),
    m_texture()
{
    dump_info();
}

RayTraceGPU::~RayTraceGPU()
{
    if (queue)
    {
        clFinish(queue);
        clReleaseCommandQueue(queue);
        queue = NULL;
    }
    if (m_lights_dev)
    {
        clReleaseMemObject(m_lights_dev);
        m_lights_dev = NULL;
    }
    if (m_primitives_dev)
    {
        clReleaseMemObject(m_primitives_dev);
        m_primitives_dev = NULL;
    }
    if (kernel_image2d)
    {
        clReleaseKernel(kernel_image2d);
        kernel_image2d = NULL;
    }
    if (program)
    {
        clReleaseProgram(program);
        program = NULL;
    }
    if (context)
    {
        clReleaseContext(context);
        context = NULL;
    }
    foreach (GLuint texture_id, m_texture)
    {
        glDeleteTextures(1, &texture_id);
    }
}

bool RayTraceGPU::init(QGLWidget * glwidget)
{
    cl_int rv = CL_SUCCESS;
    error_log.clear();

    //platforms
    QVector<cl_platform_id> platforms = get_platform_list();
    if (platforms.isEmpty())
    {
        error_log = "No platforms";
        return false;
    }

    //setup an OpenGL shared context
    int idx = 0;
    cl_context_properties properties[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    if (glwidget)
    {
        glwidget->makeCurrent();
#ifdef WIN32
        properties[idx++] = CL_GL_CONTEXT_KHR; properties[idx++] = reinterpret_cast<cl_context_properties>(wglGetCurrentContext());
        properties[idx++] = CL_WGL_HDC_KHR;    properties[idx++] = reinterpret_cast<cl_context_properties>(wglGetCurrentDC());
#endif
#ifdef BUILD_UNIX
        properties[idx++] = CL_GL_CONTEXT_KHR;  properties[idx++] = reinterpret_cast<cl_context_properties>(glXGetCurrentContext());
        properties[idx++] = CL_GLX_DISPLAY_KHR; properties[idx++] = reinterpret_cast<cl_context_properties>(display);
#endif
    }

    cl_device_id device = NULL;
    for (int p=0; p<platforms.size() && !context; p++)
    {
        cl_platform_id platform = platforms.at(p);

        properties[idx+0] = CL_CONTEXT_PLATFORM; 
        properties[idx+1] = reinterpret_cast<cl_context_properties>(platform);
        properties[idx+2] = NULL;

        //devices
        QVector<cl_device_id> devices = get_device_list(platform, CL_DEVICE_TYPE_GPU);
        for (int d=0; d<devices.size() && !context; d++)
        {
            device = devices.at(d);
            context = clCreateContext(properties, CL_DEVICE_TYPE_DEFAULT, &device, NULL, NULL, &rv);
            if (rv!=CL_SUCCESS)
            {
                context = NULL;
            }
        }

        if (context)
        {
            gpu_description = get_platform_name(platform)+" - "+get_device_name(device);
        }
    }

    if (glwidget)
    {
        glwidget->doneCurrent();
    }
    if (!context)
    {
        error_log = "Cannot create OpenCL context";
        return false;
    }

    //load the program
    QFile file(":/raycast.cl");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        error_log = QString("program load failed");
        return false;
    }
    QByteArray byteArray = file.readAll();
    char * programBuffer = byteArray.data();
    size_t programSize = byteArray.length();
    file.close();

    // create and build program
    program = clCreateProgramWithSource(context, 1, (const char**)&programBuffer, &programSize, &rv);
    CL_ENSURE_SUCCESS(rv, false, "clCreateProgramWithSource failed");

    rv = clBuildProgram(program, 1, &device, "-Werror -cl-std=CL1.1", NULL, NULL);
    if (rv)
    {   //compile error
        char build_log[1024];
        size_t size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 1024, build_log, &size);
        error_log = "clBuildProgram failed\n" + QString(build_log);
        return false;
    }

    // create kernel and command queue
    kernel_image2d = clCreateKernel(program, "raycast_image2d", &rv);
    CL_ENSURE_SUCCESS(rv, false, "clCreateKernel kernel_image2d failed");
    queue = clCreateCommandQueue(context, device, 0, &rv);
    CL_ENSURE_SUCCESS(rv, false, "clCreateCommandQueue failed");

    return true;
}

void RayTraceGPU::dump_info(void)
{
    //query all platforms
    QVector<cl_platform_id> platforms = get_platform_list();

    std::cerr << "CL DUMP: platform_count = " << platforms.size() << std::endl;

    for (int p=0; p<platforms.size(); p++)
    {
        QString platform_name = get_platform_name(platforms.at(p));
        std::cerr << "CL DUMP: platform = " << p << " name = " << platform_name.toStdString() << std::endl;

        //devices
        QVector<cl_device_id> devices = get_device_list(platforms.at(p), CL_DEVICE_TYPE_GPU);

        std::cerr << "CL DUMP: platform = " << p << " device_count = " << devices.size() << std::endl;
        for (int d=0; d<devices.size(); d++)
        {
            QString device_name = get_device_name(devices[d]);
            std::cerr << "CL DUMP: platform = " << p << " device = " << d << " name = " << device_name.toStdString() << std::endl;
        }
    }
}

QVector<cl_platform_id> RayTraceGPU::get_platform_list(void)
{
    QVector<cl_platform_id> list;
    cl_int rv = CL_SUCCESS;
    cl_uint count = 0;

    rv = clGetPlatformIDs(0, NULL, &count);
    if (rv!=CL_SUCCESS || !count)
    {
        return list;
    }

    list.resize(count);
    rv = clGetPlatformIDs(list.size(), list.data(), NULL);
    if (rv!=CL_SUCCESS)
    {
        list.clear();
    }

    return list;
}

QVector<cl_device_id> RayTraceGPU::get_device_list(cl_platform_id platform, cl_device_type type)
{
    QVector<cl_device_id> list;
    cl_int rv = CL_SUCCESS;
    cl_uint count = 0;

    rv = clGetDeviceIDs(platform, type, 0, NULL, &count);
    if (rv!=CL_SUCCESS || !count)
    {
        return list;
    }

    list.resize(count);
    rv = clGetDeviceIDs(platform, type, list.size(), list.data(), NULL);
    if (rv!=CL_SUCCESS)
    {
        list.clear();
    }

    return list;
}

QString RayTraceGPU::get_platform_name(cl_platform_id platform)
{
    cl_int rv = CL_SUCCESS;
    size_t size = 0;
    
    rv = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &size);
    if (rv!=CL_SUCCESS || !size)
    {
        return QString();
    }

    QByteArray name;
    name.resize(static_cast<int>(size)+1);

    rv = clGetPlatformInfo(platform, CL_PLATFORM_NAME, name.size(), name.data(), NULL);
    if (rv!=CL_SUCCESS)
    {
        return QString();
    }
    name.data()[size] = '\0';

    return name;
}

QString RayTraceGPU::get_device_name(cl_device_id device)
{
    cl_int rv = CL_SUCCESS;
    size_t size = 0;
    
    rv = clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &size);
    if (rv!=CL_SUCCESS || !size)
    {
        return QString();
    }

    QByteArray name;
    name.resize(static_cast<int>(size)+1);

    rv = clGetDeviceInfo(device, CL_DEVICE_NAME, name.size(), name.data(), NULL);
    if (rv!=CL_SUCCESS)
    {
        return QString();
    }
    name.data()[size] = '\0';

    return name;
}

void RayTraceGPU::render(Camera const& camera, GLuint texture_id)
{
    if (!kernel_image2d)
    {   //no kernel, abort
        return;
    }

    if (!m_scene)
    {
        return;
    }

    //opencl return value
    cl_int err = CL_SUCCESS;

    //make an opencl image from the opengl texture
    cl_mem canvas_dev = clCreateFromGLTexture2D(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture_id, &err);
    if (err!=CL_SUCCESS) { return; }

    // acquire object
    glFinish();
    err = clEnqueueAcquireGLObjects(queue, 1, &canvas_dev, 0, NULL, NULL);
    if (err!=CL_SUCCESS) { return; }

    // read image size
    size_t width = 0;
    size_t height = 0;
    err = clGetImageInfo(canvas_dev, CL_IMAGE_WIDTH, sizeof(size_t), &width, NULL);
    err = clGetImageInfo(canvas_dev, CL_IMAGE_HEIGHT, sizeof(size_t), &height, NULL);

    if (!width || !height)
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
    Matrix4x4 Minvd = M.getInverse()*K;
    Vector4d pd = Minvd*Vector4(0.0, 0.0, 0.0, 1.0);
    pd /= pd.w;

    //lights
    QVector<Scene::LightData> const& lights = m_scene->getLights();
    int light_count = lights.size();

    //global data
    Vector4f global_data = m_scene->getGlobal();

    //shapes
    QList<Scene::ScenePrimitive> const& primitives = m_scene->getPrimitives();
    int primitive_count = primitives.size();
    QVector<ScenePrimitive> primitiveVector;
    primitiveVector.resize(primitive_count);
    QVector<GLuint> textures;
    for (int i=0; i<primitive_count; i++)
    {
        Scene::ScenePrimitive const& pscn = primitives.at(i);
        ScenePrimitive & pvec = primitiveVector[i];

        if (m_scene_changed)
        {
            pvec.T = Matrix4x4f(pscn.T.a, pscn.T.b, pscn.T.c, pscn.T.d,
                                pscn.T.e, pscn.T.f, pscn.T.g, pscn.T.h,
                                pscn.T.i, pscn.T.j, pscn.T.k, pscn.T.l,
                                pscn.T.m, pscn.T.n, pscn.T.o, pscn.T.p);
        
            pvec.cAmbient = Vector4f(pscn.material.cAmbient.r, pscn.material.cAmbient.g, pscn.material.cAmbient.b, pscn.material.cAmbient.a);
            pvec.cAmbient.data[0] = pscn.material.cAmbient.r; pvec.cAmbient.data[1] = pscn.material.cAmbient.g; pvec.cAmbient.data[2] = pscn.material.cAmbient.b; pvec.cAmbient.data[3] = pscn.material.cAmbient.a;
            pvec.cDiffuse = Vector4f(pscn.material.cDiffuse.r, pscn.material.cDiffuse.g, pscn.material.cDiffuse.b, pscn.material.cDiffuse.a);
            pvec.cSpecular = Vector4f(pscn.material.cSpecular.r, pscn.material.cSpecular.g, pscn.material.cSpecular.b, pscn.material.cSpecular.a);
            pvec.cReflective = Vector4f(pscn.material.cReflective.r, pscn.material.cReflective.g, pscn.material.cReflective.b, pscn.material.cReflective.a);
            pvec.cTransparent = Vector4f(pscn.material.cTransparent.r, pscn.material.cTransparent.g, pscn.material.cTransparent.b, pscn.material.cTransparent.a);
            pvec.shininess = pscn.material.shininess;

            pvec.shape = pscn.shape;

            pvec.mu = Vector4f(pscn.material.mu.r, pscn.material.mu.g, pscn.material.mu.b, pscn.material.mu.a);

            if (pscn.shape==PRIMITIVE_JULIA)
            {   //quaternion
                float fStepSize = 0.05f;
                pvec.mu.x += m_fractal_mu.x*fStepSize;
                pvec.mu.y += m_fractal_mu.y*fStepSize;
                pvec.mu.z += m_fractal_mu.z*fStepSize;
                pvec.mu.w -= m_fractal_mu.w*fStepSize;
            }
            else if (pscn.shape==PRIMITIVE_MANDELBULB)
            {
                float epsilonStep = 0.001f;
                pvec.mu.x = std::max(0.001f, m_fractal_mu.x*epsilonStep + 0.003f); //epsilon
                pvec.mu.y = m_fractal_mu.y/150.0f + 0.2f; //MR2  0.2 +/- 0.1667
            }
            else if (pscn.shape==PRIMITIVE_MANDELBOX)
            {
                float epsilonStep = 0.001f;
                pvec.mu.x = m_fractal_mu.x/25.0f + 2.2f; //SCALE 1.2 - 2.2
                pvec.mu.y = m_fractal_mu.y/150.0f + 0.2f; //MR2  0.2 +/- 0.1667
                pvec.mu.z = std::max(0.001f, m_fractal_mu.z*epsilonStep + 0.003f); //epsilon
            }
        }

        GLuint texture_id;
        if (pscn.material.textureMap && pscn.material.textureMap->isUsed 
            && get_texture(QString::fromStdString(pscn.material.textureMap->filename), texture_id))
        {
            pvec.texture_id = textures.indexOf(texture_id);
            if (pvec.texture_id<0)
            {
                textures.append(texture_id);
                pvec.texture_id = textures.size()-1;
            }
            pvec.blend = pscn.material.blend;
            pvec.repeat = Vector2f(pscn.material.textureMap->repeatU, pscn.material.textureMap->repeatV);
        }
        else
        {
            pvec.texture_id = GL_INVALID_VALUE;
            pvec.blend = 0.0f;
            pvec.repeat = Vector2f(0.0f, 0.0f);
        }
    }


    /// GPU call -----------------------

    // allocate device memory
    if (m_scene_changed)
    {
        m_lights_dev = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, light_count*sizeof(Scene::LightData), const_cast<Scene::LightData*>(lights.data()), &err);
        m_light_count = light_count;
        m_primitives_dev = clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, primitive_count*sizeof(ScenePrimitive), primitiveVector.data(), &err);
        m_primitive_count = primitive_count;
        m_scene_changed = false;
    }

    cl_image_format image_format; image_format.image_channel_order = CL_RGBA; image_format.image_channel_data_type = CL_UNSIGNED_INT8;
    cl_mem dummy_img = clCreateImage2D(context, CL_MEM_READ_ONLY, &image_format, 1, 1, 0, NULL, &err);
    cl_mem texture_dev[4] = {dummy_img, dummy_img, dummy_img, dummy_img};
    for (int i=0; i<4 && i<textures.size(); i++)
    {
        texture_dev[i] = clCreateFromGLTexture2D(context, CL_MEM_READ_ONLY, GL_TEXTURE_2D, 0, textures.at(i), &err);
        err = clEnqueueAcquireGLObjects(queue, 1, texture_dev+i, 0, NULL, NULL);
    }

    Vector4f p(pd.x, pd.y, pd.z, pd.w);
    Matrix4x4f Minv(Minvd.a, Minvd.b, Minvd.c, Minvd.d,
                    Minvd.e, Minvd.f, Minvd.g, Minvd.h,
                    Minvd.i, Minvd.j, Minvd.k, Minvd.l,
                    Minvd.m, Minvd.n, Minvd.o, Minvd.p);

    //set the arguments
    int arg = 0;
    err = clSetKernelArg(kernel_image2d, arg++,  sizeof(cl_mem), &canvas_dev);
    err = clSetKernelArg(kernel_image2d, arg++, 4*sizeof(float), &global_data.data);
    err = clSetKernelArg(kernel_image2d, arg++,  sizeof(cl_mem), &m_lights_dev);
    err = clSetKernelArg(kernel_image2d, arg++,  sizeof(cl_int), &m_light_count);
    err = clSetKernelArg(kernel_image2d, arg++,  sizeof(cl_mem), &m_primitives_dev);
    err = clSetKernelArg(kernel_image2d, arg++,  sizeof(cl_int), &m_primitive_count);
    err = clSetKernelArg(kernel_image2d, arg++, 4*sizeof(float), &p.data);
    err = clSetKernelArg(kernel_image2d, arg++,16*sizeof(float), &Minv.data);

    err = clSetKernelArg(kernel_image2d, arg++, sizeof(cl_mem), texture_dev+0);
    err = clSetKernelArg(kernel_image2d, arg++, sizeof(cl_mem), texture_dev+1);
    err = clSetKernelArg(kernel_image2d, arg++, sizeof(cl_mem), texture_dev+2);
    err = clSetKernelArg(kernel_image2d, arg++, sizeof(cl_mem), texture_dev+3);


    // execute kernel, read back the output and print to screen
    size_t dims[2] = {width, height};
    err = clEnqueueNDRangeKernel(queue, kernel_image2d, 2, NULL, dims, NULL, 0, NULL, NULL);

    // free the object
    err = clEnqueueReleaseGLObjects(queue, 1, &canvas_dev, 0, NULL, NULL);
    for (int i=0; i<4 && i<textures.size(); i++)
    {
        err = clEnqueueReleaseGLObjects(queue, 1, texture_dev+i, 0, NULL, NULL);
    }

    // finish
    err = clFlush(queue);
    err = clFinish(queue);
    err = clReleaseMemObject(canvas_dev);
    for (int i=0; i<4 && i<textures.size(); i++)
    {
        err = clReleaseMemObject(texture_dev[i]);
    }
    err = clReleaseMemObject(dummy_img);
}

bool RayTraceGPU::get_texture(QString const& filename, GLuint & texture_id)
{
    QMap<QString,GLuint>::ConstIterator iter = m_texture.constFind(filename);
    if (iter!=m_texture.constEnd())
    {   //found
        texture_id = *iter;
        return (*iter!=GL_INVALID_VALUE);
    }

    //not found: try to load
    if (load_texture(filename))
    {   //loaded retry
        iter = m_texture.constFind(filename);
        if (iter!=m_texture.constEnd())
        {   //found
            texture_id = *iter;
        return (*iter!=GL_INVALID_VALUE);
        }
    }

    //failed: add the key anyway
    m_texture.insert(filename, GL_INVALID_VALUE);

    return false;
}

bool RayTraceGPU::load_texture(QString const& filename)
{
    //QMutexLocker locker(&m_lock);
    QMap<QString,GLuint>::ConstIterator iter = m_texture.constFind(filename);
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
            std::cerr << "[RayTraceGPU] (win) Image loaded FAILED: " << winFile.fileName().toStdString() << std::endl;
#endif
            std::cerr << "[RayTraceGPU] Image loaded FAILED: " << filename.toStdString() << std::endl;
            return false;
#ifdef _MSC_VER
        }//if
#endif
    }

    //copy to OpenGL
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    std::cerr << "[RayTraceGPU] Image loaded: (id=" << texture_id << ") " << filename.toStdString() << std::endl;

    //bind
    glBindTexture(GL_TEXTURE_2D, texture_id);

    //copy
    QImage gl_compatible_image = QGLWidget::convertToGLFormat(image);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, gl_compatible_image.width(), gl_compatible_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, gl_compatible_image.bits());

    //texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //unbind
    glBindTexture(GL_TEXTURE_2D, 0);

    m_texture.insert(filename, texture_id);
    return true;
}