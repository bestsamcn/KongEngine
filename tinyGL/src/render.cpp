#include "render.h"

#include "Camera.h"
#include "Engine.h"
#include "model.h"
#include "tgaimage.h"
#include "message.h"

using namespace tinyGL;
using namespace glm;
using namespace std;

int CRender::Init()
{
	render_window = Engine::GetRenderWindow();
	InitCamera();

	// ��ʼ����պ�
	m_SkyBox.Init();
	//init show map
	glGenFramebuffers(1, &m_FrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);

	// ? �����ͼ���
	glGenTextures(1, &m_DepthTexture);
	glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);
	// ���buffer
	glDrawBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return -1;

	// ����shader�����벢������Ӧ�ĳ���
	m_ShadowMapProgramID = LoadShaders("../../../../resource/shader/shadowmap_vert.shader",
		"../../../../resource/shader/shadowmap_frag.shader");

	m_DepthMatrixID = glGetUniformLocation(m_ShadowMapProgramID, "depth_mvp");
	return 0;
}

int CRender::InitCamera()
{
	mainCamera = new CCamera(vec3(-4.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

	mainCamera->InitControl();
	return 0;
}

int CRender::Update(double delta)
{
	UpdateLightDir(delta);
	//update camera
	mainCamera->Update(delta);		
	
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBuffer);
	glViewport(0, 0, 1024, 1024); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	// for (auto& render_info : m_vRenderInfo)
	// {
	// 	RenderShadowMap(render_info);
	// }

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//opengl32.dll	
	glViewport(0, 0, 1024, 768); // Render on the whole framebuffer, complete from the lower left corner to the upper right

	RenderSkyBox();
	for (auto& render_info : m_vRenderInfo)
	{
		RenderModel(render_info);
	}
	
	// Swap buffers
	glfwSwapBuffers(render_window);
	glfwPollEvents();
	return 1;
}

void CRender::AddRenderInfo(SRenderInfo render_info, const std::string shader_paths[2])
{
	render_info._program_id = LoadShaders(shader_paths[0], shader_paths[1]);
	m_vRenderInfo.push_back(render_info);
}


void CRender::RenderSkyBox()
{
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrixNoTranslate(); //
	m_SkyBox.Render(mvp);
}

void CRender::RenderShadowMap(const SRenderInfo& render_info)
{		
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); // Cull back-facing triangles -> draw only front-facing triangles

	// ʹ����Ӱ��ͼ��shader
	glUseProgram(m_ShadowMapProgramID);	

	// Compute the MVP matrix from the light's point of view
	//1024.0f / 768.0f, 0.1f, 500.0f
	mat4 depth_proj_mat = ortho<float>(-10, 10, -10, 10, -10, 20);
	mat4 depth_view_mat = lookAt(m_LightDir+m_LightPos, m_LightPos, glm::vec3(0, 1, 0));

	mat4 depth_model_mat = mat4(1.0);
	m_DepthMVP = depth_proj_mat * depth_view_mat * depth_model_mat;

	// in the "MVP" uniform
	glUniformMatrix4fv(m_DepthMatrixID, 1, GL_FALSE, &m_DepthMVP[0][0]);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, render_info.vertexBuffer);
	glVertexAttribPointer(
		0,  // The attribute we want to configure
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);	

	glDrawArrays(GL_TRIANGLES, 0, render_info._vertex_size / 3); // Starting from vertex 0; 3 vertices total -> 1 triangle	

	glDisableVertexAttribArray(0);
}

void CRender::RenderModel(const SRenderInfo& render_info) const
{	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(render_info._program_id);
	glBindVertexArray(render_info.vertexArrayId);	// ��VAO
	
	mat4 Model = mat4(1.0f);
	mat4 projection = mainCamera->GetProjectionMatrix();
	mat4 mvp = projection * mainCamera->GetViewMatrix() * Model; //
	GLuint matrix_id = glGetUniformLocation(render_info._program_id, "MVP");
	glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

	GLuint light_shininess_id = glGetUniformLocation(render_info._program_id, "shininess");
	glUniform1f(light_shininess_id, render_info._material._shininess);

	GLuint cam_pos_id = glGetUniformLocation(render_info._program_id, "cam_pos");
	glUniform3fv(cam_pos_id, 1, &mainCamera->GetPosition()[0]);

	GLuint light_dir_id = glGetUniformLocation(render_info._program_id, "light_dir");
	glUniform3fv(light_dir_id, 1, &m_LightDir[0]);
	
	GLuint light_color_id = glGetUniformLocation(render_info._program_id, "light_color");
	glUniform3fv(light_color_id, 1, &m_LightColor[0]);

	//use shadow map
	mat4 bias_mat(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0
	);

	mat4 depth_bias_mvp = bias_mat * m_DepthMVP;

	GLuint depth_bias_id = glGetUniformLocation(render_info._program_id, "depth_bias_mvp");
	glUniformMatrix4fv(depth_bias_id, 1, GL_FALSE, &depth_bias_mvp[0][0]);

	GLuint model_mat_id = glGetUniformLocation(render_info._program_id, "normal_model_mat");

	/*
	 * ���߾��󱻶���Ϊ��ģ�;������Ͻ�3x3���ֵ�������ת�þ��󡹡�
	 * ��ʹ����һЩ���Դ����Ĳ������Ƴ��Է������������ŵ�Ӱ�졣
	 */
	mat3 normal_model_mat = transpose(inverse(Model));
	glUniformMatrix3fv(model_mat_id, 1, GL_FALSE, &normal_model_mat[0][0]);
	
	// Draw the triangle !
	// if no ido, use draw array
	if(render_info.indexBuffer == GL_NONE)
	{
		glDrawArrays(GL_TRIANGLES, 0, render_info._vertex_size / render_info._stride_count); // Starting from vertex 0; 3 vertices total -> 1 triangle	
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_info.indexBuffer);
		glDrawElements(GL_TRIANGLES, render_info._indices_count, GL_UNSIGNED_INT, 0);
	}
	
	glBindVertexArray(GL_NONE);	// ���VAO
}

void CRender::UpdateLightDir(float delta)
{
	// ���ձ仯
	double light_speed = 30.0;

	light_yaw += light_speed*delta;
	
	vec3 front;
	front.x = cos(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));
	front.y = sin(glm::radians(light_pitch));
	front.z = sin(glm::radians(light_yaw)) * cos(glm::radians(light_pitch));

	m_LightDir = normalize(front);
	// printf("--- light dir %f %f %f\n", m_LightDir.x, m_LightDir.y, m_LightDir.z);
}


GLuint CRender::LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path)
{
	// Create the shaders
	GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string vs_code;
	std::ifstream vs_stream(vertex_file_path, std::ios::in);
	if (vs_stream.is_open()) {
		std::stringstream sstr;
		sstr << vs_stream.rdbuf();
		vs_code = sstr.str();
		vs_stream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path.c_str());
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string fs_code;
	std::ifstream fs_stream(fragment_file_path, std::ios::in);
	if (fs_stream.is_open()) {
		std::stringstream sstr;
		sstr << fs_stream.rdbuf();
		fs_code = sstr.str();
		fs_stream.close();
	}

	GLint result = GL_FALSE;
	int info_log_length;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path.c_str());
	char const * vs_ptr = vs_code.c_str();
	glShaderSource(vs_id, 1, &vs_ptr, NULL);
	glCompileShader(vs_id);

	// Check Vertex Shader
	glGetShaderiv(vs_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vs_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> vs_error_msg(info_log_length + 1);
		glGetShaderInfoLog(vs_id, info_log_length, NULL, &vs_error_msg[0]);
		printf("%s\n", &vs_error_msg[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path.c_str());
	char const * fs_ptr = fs_code.c_str();
	glShaderSource(fs_id, 1, &fs_ptr, NULL);
	glCompileShader(fs_id);

	// Check Fragment Shader
	glGetShaderiv(fs_id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fs_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> fs_error_msg(info_log_length + 1);
		glGetShaderInfoLog(fs_id, info_log_length, NULL, &fs_error_msg[0]);
		printf("%s\n", &fs_error_msg[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint prog_id = glCreateProgram();
	glAttachShader(prog_id, vs_id);
	glAttachShader(prog_id, fs_id);
	glLinkProgram(prog_id);

	// Check the program
	glGetProgramiv(prog_id, GL_LINK_STATUS, &result);
	glGetProgramiv(prog_id, GL_INFO_LOG_LENGTH, &info_log_length);
	if (info_log_length > 0) {
		std::vector<char> prog_error_msg(info_log_length + 1);
		glGetProgramInfoLog(prog_id, info_log_length, NULL, &prog_error_msg[0]);
		printf("%s\n", &prog_error_msg[0]);
	}

	glDetachShader(prog_id, vs_id);
	glDetachShader(prog_id, fs_id);

	glDeleteShader(vs_id);
	glDeleteShader(fs_id);

	return prog_id;
}
