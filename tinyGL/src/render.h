#pragma once
#include "Component/CameraComponent.h"
#include "Common.h"
#include "postprocess.h"
#include "skybox.h"
#include "Component/Mesh/Terrain.h"
#include "Component/Mesh/VolumetricCloud.h"
#include "Shader/PBRShader.h"
#include "Shader/DeferInfoShader.h"
#include "Shader/Shader.h"

namespace Kong
{
	class FinalPostprocessShader;
	class CPointLightComponent;
	class CDirectionalLightComponent;
	class CMeshComponent;
	class CModelMeshComponent;
	class CCamera;

	// 针对场景中的所有渲染物，使用UBO存储基础数据优化性能
	class UBOHelper
	{
	public:
		template <class T>
		void AppendData(T data, const std::string& name);

		template <class T>
		void UpdateData(const T& data, const std::string& name) const;
		
		void Init(GLuint in_binding);
		// 开始绑定
		void Bind() const;
		// 结束绑定
		void EndBind() const;
	private:
		std::map<string, unsigned> data_offset_cache;
		size_t next_offset = 0;
		GLuint binding = GL_NONE;
		GLuint ubo_idx = GL_NONE;
	};

	template <class T>
	void UBOHelper::AppendData(T data, const std::string& name)
	{
		data_offset_cache.emplace(name, next_offset);
		//UpdateStd140Offset(data);
		size_t size = sizeof(T);
		next_offset += size;
	}

	template <class T>
	void UBOHelper::UpdateData(const T& data, const std::string& name) const
	{
		auto find_iter = data_offset_cache.find(name);
		if(find_iter == data_offset_cache.end())
		{
			assert(false, "update data failed");
			return;
		}

		unsigned offset = find_iter->second;
		size_t size = sizeof(T);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, &data);
	}

	// 延迟渲染的数据结构
	struct DeferBuffer
	{
		GLuint g_buffer_	= 0;
		GLuint g_position_	= 0;
		GLuint g_normal_	= 0;
		GLuint g_albedo_	= 0;
		GLuint g_orm_		= 0;	//o:ao, r:roughness, m: metallic
		GLuint g_rbo_		= 0;
		
		void Init(unsigned width, unsigned height);
		void GenerateDeferRenderTextures(int width, int height);
		// 延迟渲染着色器
		shared_ptr<DeferredBRDFShader> defer_render_shader;
	};

	struct SSAOHelper
	{
		// SSAO相关
		unsigned ssao_kernel_count = 64;
		unsigned ssao_noise_size = 4;
		vector<glm::vec3> ssao_kernal_samples;
		vector<glm::vec3> ssao_kernal_noises;
		
		GLuint ssao_fbo = GL_NONE;
		GLuint SSAO_BlurFBO = GL_NONE;
		GLuint ssao_noise_texture = GL_NONE;
		GLuint ssao_result_texture = GL_NONE;
		GLuint ssao_blur_texture = GL_NONE;
		shared_ptr<SSAOShader> ssao_shader_;
		shared_ptr<Shader> ssao_blur_shader_;

		void Init(int width, int height);
		void GenerateSSAOTextures(int width, int height);
	};
	
	class CRender
	{
	public:
		static CRender* GetRender();
		static GLuint GetNullTexId();
		static glm::vec2 GetNearFar();
		static shared_ptr<CQuadShape> GetScreenShape();
		
		GLFWwindow* render_window;
		CRender() = default;

		GLuint GetSkyboxTexture() const;
		GLuint GetSkyboxDiffuseIrradianceTexture() const;
		GLuint GetSkyboxPrefilterTexture() const;
		GLuint GetSkyboxBRDFLutTexture() const;
		int Init();
		int Update(double delta);
		void PostUpdate();
		CCamera* GetCamera() {return mainCamera;}
		void ChangeSkybox();
		void OnWindowResize(int width, int height);
						
		PostProcess post_process;
		int render_sky_env_status = 2;
		bool use_ssao = true;
		
		// 场景光源信息
		SSceneRenderInfo scene_render_info;
	private:
		int InitCamera();
		void InitUBO();
		void RenderSkyBox();
		void RenderScene() const;
		void DeferRenderSceneToGBuffer() const;
		void DeferRenderSceneLighting() const;

		void SSAORender() const;
		
		// 预先处理一下场景中的光照。目前场景只支持一个平行光和四个点光源，后续需要根据object的位置等信息映射对应的光源
		void CollectLightInfo();
		void RenderSceneObject();
		
		void RenderShadowMap();
	private:
		
		CSkyBox m_SkyBox;
		GLuint null_tex_id			= GL_NONE;
		
		shared_ptr<Shader> shadowmap_debug_shader;
		GLuint m_QuadVAO = GL_NONE;
		GLuint m_QuadVBO = GL_NONE;

		CCamera* mainCamera = nullptr;
		/* 矩阵UBO，保存场景基础的矩阵信息
		 *  mat4 model
		 *  mat4 view
		 *  mat4 projection
		 *  vec3 cam_pos;
		 */
		UBOHelper matrix_ubo;

		// 光照UBO，保存场景基础的光照信息
		/*		
		 *	SceneLightInfo light_info
		 */
		UBOHelper scene_light_ubo;

		// 延迟渲染
		DeferBuffer defer_buffer_;
		SSAOHelper ssao_helper_;

		shared_ptr<CQuadShape> quad_shape;
	};
}
