#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H 1

#include <Windows.h>
#define VK_USE_PLATFORM_WIN32_KHR 1
#pragma comment(linker, "/subsystem:windows")

#include <vulkan/vulkan.h>
#include <string>
#include <atomic>


class Swapchain;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class EngineWindow {

public:
	EngineWindow();
	~EngineWindow();
	EngineWindow(int x, int y, int width, int height);
	void Initialize(HINSTANCE);
	void Redraw();
	void Update();
	void InitializeSurface(VkInstance);
	void SetUpSwapChain(Swapchain*);
	VkSurfaceKHR GetSurface(){ return m_TheVulkanPresentSurface; }


private:
	std::string name;
	std::atomic<bool> mInitialized;
	int m_width;
	int m_height;
	int x_offset;
	int y_offset;
	HWND m_handle;
	HINSTANCE m_WindowsAppInstance;

	Swapchain * mSwapchain;

	VkInstance		m_InstanceForSurface;
	VkSurfaceKHR	m_TheVulkanPresentSurface;
};
#endif WINDOWS_WINDOW_H