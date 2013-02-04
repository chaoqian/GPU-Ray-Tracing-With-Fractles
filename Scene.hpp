
#ifndef __SCENE_HPP__
#define __SCENE_HPP__

#include "math/CS123Algebra.h"
#include "lib/CS123SceneData.h"

class CS123ISceneParser;

class Scene
{
public:
    class ScenePrimitive
    {
    public:
        ScenePrimitive();
        ~ScenePrimitive();

        Matrix4x4 T;
        PrimitiveType shape;
        CS123SceneMaterial material;
    };

    class LightData
    {
    public:
        Vector4f color;
        Vector4f attenuation;
        Vector4f location;
        Vector4f spot;

        LightData() : color(0.f,0.f,0.f,0.f), attenuation(0.f,0.f,0.f,0.f), location(0.f,0.f,0.f,0.f) {}
    };

    Scene();
    virtual ~Scene();

    static void parse(Scene *sceneToFill, CS123ISceneParser *parser);

    inline Vector4f const& getGlobal(void) const {return m_global;}
    inline QVector<LightData> const& getLights(void) const {return m_lights;}
    inline QList<ScenePrimitive> const& getPrimitives(void) const {return m_primitives;}
    inline QList<ScenePrimitive> & getPrimitives(void) {return const_cast<Scene*>(this)->m_primitives;}

protected:
    /*! Adds a primitive to the scene. */
    virtual void addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix);

    /*! Adds a light to the scene. */
    virtual void addLight(const CS123SceneLightData &sceneLight);

    /*! Sets the global data for the scene. */
    virtual void setGlobal(const CS123SceneGlobalData &global);

    /*! Parse one scene node. */
    static void parseNode(CS123SceneNode const& node, Scene & scene, Matrix4x4 T = Matrix4x4::identity());

    Vector4f              m_global;
    QVector<LightData>    m_lights;
    QList<ScenePrimitive> m_primitives;
};

#endif // __SCENE_HPP__