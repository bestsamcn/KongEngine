#include "ShadowMapShader.h"

#include "Scene.h"

using namespace Kong;
using namespace glm;

void DirectionalLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap.frag")}
    };
    
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");
}

void DirectionalLightCSMShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/csm.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/csm.frag")},
        {gs, CSceneLoader::ToResourcePath("shader/shadow/csm.geom")}
    };
    
    shader_id = LoadShaders(shader_path_map);
    assert(shader_id, "Shader load failed!");
}

void PointLightShadowMapShader::InitDefaultShader()
{
    shader_path_map = {
        {vs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.vert")},
        {fs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.frag")},
        {gs, CSceneLoader::ToResourcePath("shader/shadow/shadowmap_pointlight.geom")}
    };
    shader_id = LoadShaders(shader_path_map);

    assert(shader_id, "Shader load failed!");

}