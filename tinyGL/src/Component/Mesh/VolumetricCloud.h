#pragma once
#include "MeshComponent.h"

namespace Kong
{
    class CloudModel
    {
    public:
        CloudModel();
        void GenerateWeatherMap();
        
    private:
        friend class VolumetricCloud;
        friend class CSkyBox;
        
        shared_ptr<Shader> perlin_worley_comp_shader;
        shared_ptr<Shader> worley_comp_shader;
        shared_ptr<Shader> weather_compute_shader;
        // control parameter
        float coverage = 0.45f, cloud_speed = 450.f, crispiness = 40.f, curliness = 0.1f, density = 0.02f, absorption = 0.35f;
        float earth_radius = 600000.0f, sphere_inner_radius = 5000.0f, sphere_outer_radius = 17000.0f;
        float perlin_frequency = 0.8f;
        bool enable_god_rays = false;
        bool enable_powder = false;
        bool post_process = true;

        glm::vec3 cloud_color_top = glm::vec3(169.f, 149.f, 149.f) * (1.5f/255.f);
        glm::vec3 cloud_color_bottom = glm::vec3(65.f, 70.f, 80.f) * (1.5f/255.f);

        glm::vec3 sky_color_top = glm::vec3(0.5, 0.7, 0.8)*1.05f;
        glm::vec3 sky_color_bottom = glm::vec3(0.9, 0.9, 0.95);
        glm::vec3 seed = glm::vec3(0), old_seed = glm::vec3(0);

        GLuint perlin_texture = 0, worley32_texture = 0, weather_texutre = 0;
    };
    
    class VolumetricCloud : public CMeshComponent
    {
    public:
        VolumetricCloud();

        void SimpleDraw() override;
        
        // cloud process
        GLuint cloud_tex, bloom_tex, alphaness_tex, depth_tex;
    private:
        friend class CSkyBox;
        
        shared_ptr<CloudModel> cloud_model_;
        shared_ptr<Shader> cloud_compute_shader_;
        
        GLuint GenerateTexture2D(unsigned w, unsigned h);
    };
}

