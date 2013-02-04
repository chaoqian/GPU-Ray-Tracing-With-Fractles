
#include "Scene.hpp"

#include <QTime>

#include "lib/CS123ISceneParser.h"

Scene::ScenePrimitive::ScenePrimitive() : shape(PRIMITIVE_NONE), material()
{
}

Scene::ScenePrimitive::~ScenePrimitive()
{
    if (material.textureMap)
    {
        delete material.textureMap;
    }
    if (material.bumpMap)
    {
        delete material.bumpMap;
    }
}

Scene::Scene() : 
    m_global(),
    m_lights(),
    m_primitives()
{
}

Scene::~Scene()
{
}

void Scene::parse(Scene *sceneToFill, CS123ISceneParser *parser)
{
    // TODO: load scene into sceneToFill using setGlobal(), addLight(), addPrimitive(), and finishParsing()

    if (!sceneToFill || !parser)
    {
        return;
    }

    QTime t;
    t.start();
    
    std::cerr << "Parsing start\n";

    CS123SceneGlobalData global;
    if (parser->getGlobalData(global))
    {
        sceneToFill->setGlobal(global);
    }

    int light_count = parser->getNumLights();
    for (int i=0; i<light_count; i++)
    {
        CS123SceneLightData light;
        if (parser->getLightData(i, light))
        {
            sceneToFill->addLight(light);
        }
    }

    const CS123SceneNode * root = parser->getRootNode();
    if (root)
    {
        parseNode(*root, *sceneToFill);
    }

    //normalized the light vectors (just in case)
    for (QVector<LightData>::iterator light=sceneToFill->m_lights.begin(); light!=sceneToFill->m_lights.end(); light++)
    {
        if (light->location.w!=0.0)
        {
            light->location.normalize();
        }
    }

    std::cerr << "Parsing end: " << t.elapsed() << "\n";
}

void Scene::addPrimitive(const CS123ScenePrimitive &scenePrimitive, const Matrix4x4 &matrix)
{
    //append node
    m_primitives.append(ScenePrimitive());
    ScenePrimitive & primitive = m_primitives.last();

    //get shape
    primitive.shape = scenePrimitive.type;

    if (!primitive.shape)
    {   //failed to create primitive shape
        m_primitives.removeLast();
        return;
    }

    //copy materials
    //primitive.material = scenePrimitive.material;
    memcpy(&(primitive.material), &(scenePrimitive.material), sizeof(CS123SceneMaterial));
    primitive.material.textureMap = NULL;
    primitive.material.bumpMap = NULL;
    if (scenePrimitive.material.textureMap)
    {
        primitive.material.textureMap = new CS123SceneFileMap();
        (*primitive.material.textureMap) = (*scenePrimitive.material.textureMap);
    }

    primitive.T = matrix;
}

void Scene::addLight(const CS123SceneLightData &sceneLight)
{
    LightData data;

    if (sceneLight.type==LIGHT_POINT)
    {
        data.location.x = sceneLight.pos.x;
        data.location.y = sceneLight.pos.y;
        data.location.z = sceneLight.pos.z;
        data.location.w = sceneLight.pos.w;
        data.spot = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else  if (sceneLight.type==LIGHT_DIRECTIONAL)
    {
        data.location.x = sceneLight.dir.x;
        data.location.y = sceneLight.dir.y;
        data.location.z = sceneLight.dir.z;
        data.location.w = sceneLight.dir.w;
        data.location.normalize(); //direction vector
        data.spot = Vector4f(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else if (sceneLight.type==LIGHT_SPOT)
    {
        data.location.x = sceneLight.pos.x;
        data.location.y = sceneLight.pos.y;
        data.location.z = sceneLight.pos.z;
        data.location.w = sceneLight.pos.w;
        data.spot.x = sceneLight.dir.x;
        data.spot.y = sceneLight.dir.y;
        data.spot.z = sceneLight.dir.z;
        data.spot.w = 0.0f;
        data.spot.normalize(); //direction vector
        data.spot.w = sceneLight.penumbra;
    }
    else
    {   //unsupported type
        return;
    }

    data.color.x = sceneLight.color.r;
    data.color.y = sceneLight.color.g;
    data.color.z = sceneLight.color.b;
    data.color.w = sceneLight.color.a;

    data.attenuation.x = sceneLight.function.x;
    data.attenuation.y = sceneLight.function.y;
    data.attenuation.z = sceneLight.function.z;
    data.attenuation.w = 0.f;

    m_lights.push_back(data);
}

void Scene::setGlobal(const CS123SceneGlobalData &global)
{
    m_global.x = global.ka;
    m_global.y = global.kd;
    m_global.z = global.ks;
    m_global.w = global.kt;
}

void Scene::parseNode(CS123SceneNode const& node, Scene & scene, Matrix4x4 T)
{
    //collect the transformations
    for (std::vector<CS123SceneTransformation*>::const_iterator iter=node.transformations.begin();
            iter!=node.transformations.end(); iter++)
    {
        const CS123SceneTransformation * st = *iter;
        if (st)
        {
            T *= st->MatrixTransform();
        }
    }

    //parse primitives
    for (std::vector<CS123ScenePrimitive*>::const_iterator iter=node.primitives.begin();
            iter!=node.primitives.end(); iter++)
    {
        const CS123ScenePrimitive * primitive = *iter;
        if (primitive)
        {
            scene.addPrimitive(*primitive, T);
        }
    }

    //parse children
    for (std::vector<CS123SceneNode*>::const_iterator iter=node.children.begin();
            iter!=node.children.end(); iter++)
    {
        const CS123SceneNode * childNode = *iter;
        if (childNode)
        {
            parseNode(*childNode, scene, T);
        }
    }
}