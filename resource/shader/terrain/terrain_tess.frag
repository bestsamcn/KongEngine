#version 450 compatibility
#extension GL_ARB_shading_language_include : require
#include "/common/common.glsl"

in vec2 frag_texcoord;
in vec3 frag_pos;
in vec3 frag_normal;

in float tes_height;
out vec4 FragColor;

in mat3 TBN;

uniform vec3 cam_pos;
uniform sampler2D grass_texture;
uniform sampler2D grass_normal_texture;
uniform sampler2D sand_texture;
uniform sampler2D sand_normal_texture;
uniform sampler2D rock_texture;
uniform sampler2D rock_normal_texture;

vec3 ApplyFog(vec3 origin_color)
{    
    vec3 cam_to_point = frag_pos - cam_pos;
    float pixel_dist = length(cam_to_point);
    vec3 cam_to_point_dir = normalize(cam_to_point);
    float fog_fall_off = 0.00005;

    // float fog_amount = 1.0 - exp(-pixel_dist*fog_fall_off);
    float fog_amount = exp(-cam_pos.y*fog_fall_off) * (1.0-exp(-pixel_dist*cam_to_point_dir.y*fog_fall_off))/cam_to_point_dir.y;
    // fog_amount = clamp(fog_amount, 0, 1);
    vec3 light_dir = light_info_ubo.directional_light.light_dir.xyz;
    float sum_amount = max(dot(cam_to_point_dir, -light_dir), 0.0);

    vec3 fog_color = mix(vec3(0.5, 0.6, 0.7), vec3(1.0, 0.9, 0.7), pow(sum_amount, 8.0));
    // vec3 fog_color = vec3(0.5, 0.6, 0.7);

    return mix(origin_color, fog_color, fog_amount);
}

int trans = 20;
void main()
{
    float grass_scale = 12.f;
    float rock_scale = 10.f;
    float sand_scale = 10.f;

    vec4 grass_color = texture(grass_texture, frag_texcoord*grass_scale);
    vec4 rock_color = texture(rock_texture, frag_texcoord*rock_scale);
    vec4 sand_color = texture(sand_texture, frag_texcoord*sand_scale);

    vec3 grass_normal = normalize(TBN * (texture(grass_normal_texture, frag_texcoord*grass_scale).xyz*2.0 - 1.0));
    vec3 sand_normal = normalize(TBN * (texture(sand_normal_texture, frag_texcoord*sand_scale).xyz*2.0 - 1.0));
    vec3 rock_normal = normalize(TBN * (texture(rock_normal_texture, frag_texcoord*rock_scale).xyz*2.0 - 1.0));

    float slope = abs(dot(frag_normal, vec3(0.0, 1.0, 0.0)));
    float grass_coverage = 0.2;
    // float blending_coeff = pow((slope - grass_coverage*0.9) / (grass_coverage*0.1), 1.0);
    float blending_coeff = (slope - grass_coverage) / (1 - grass_coverage);

    vec4 color;
    vec3 normal = frag_normal;
    float rock_height = 4*trans;
    if(tes_height <= trans)
    {
        color = sand_color;        
        normal = sand_normal;
    }
    else if(tes_height <= 2.0*trans)
    {
        float alpha = min(1, (tes_height-trans)/trans);
        color = mix(sand_color, grass_color, alpha);        
        normal = mix(sand_normal, grass_normal, alpha);
    }
    else 
    {    
        color = grass_color;
        normal = grass_normal;
    }
    
    if(slope > grass_coverage)
    {
        color = mix(rock_color, color, blending_coeff);
        normal = mix(rock_normal, normal, blending_coeff);
    }
    else
    {        
        color = rock_color;
        normal = rock_normal;
    }

    if(light_info_ubo.has_dir_light.x > 0)
    {
        vec3 light_dir = -light_info_ubo.directional_light.light_dir.xyz;
        vec4 light_color = light_info_ubo.directional_light.light_color;

        vec3 view = matrix_ubo.cam_pos.xyz - frag_pos;
        float diffuse_factor = max(0, dot(light_dir, normal));
        color = diffuse_factor * light_color * color;
    }

    vec3 final_color = ApplyFog(color.xyz);
    FragColor = vec4(final_color, 1.0);
}