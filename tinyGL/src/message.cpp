#include "message.h"
#include "render.h"

CMessage g_MessageHandler;

CMessage* GetMessageHandler()
{
	return &g_MessageHandler;
}

void CMessage::KeyCallback(GLFWwindow* window, int key, int scan_code, int action, int mods)
{
	//�Ȳ���scan code��mode
	auto key_pair = std::make_pair(key, action);
	auto key_func_pair = g_MessageHandler.m_mKeyFuncMap.find(key_pair);

	if (key_func_pair != g_MessageHandler.m_mKeyFuncMap.end())
	{
		key_func_pair->second();
	}
}

void CMessage::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//�Ȳ���mode
	auto mouse_pair = std::make_pair(button, action);
	auto mouse_func_pair = g_MessageHandler.m_mMouseFuncMap.find(mouse_pair);

	if (mouse_func_pair != g_MessageHandler.m_mMouseFuncMap.end())
	{
		mouse_func_pair->second();
	}
}

void CMessage::BindKeyToFunction(int key, int action, void(*func)())
{
	auto key_pair = std::make_pair(key, action);
	g_MessageHandler.m_mKeyFuncMap.emplace(key_pair, func);
}

void CMessage::BindMouseToFunction(int key, int action, void(*func)())
{
	auto key_pair = std::make_pair(key, action);
	g_MessageHandler.m_mMouseFuncMap.emplace(key_pair, func);
}