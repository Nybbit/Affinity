#include "window.h"
#include <plog/Log.h>

Window::Window(const std::string& window_title, glm::ivec2 resolution)
	: m_windowTitle(window_title), m_resolution(resolution)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	m_window = glfwCreateWindow(m_resolution.x, m_resolution.y, m_windowTitle.c_str(), nullptr, nullptr);

	if (!m_window)
	{
		throw std::runtime_error("Failed to create window");
	}

	glfwMakeContextCurrent(m_window);
}

Window::~Window()
{
	if (m_window)
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
}

Window::Window(Window&& other) noexcept
	: m_window(other.m_window),
	  m_windowTitle(std::move(other.m_windowTitle)),
	  m_resolution(std::move(other.m_resolution))
{
}

Window& Window::operator=(const Window& other)
{
	if (this == &other)
		return *this;
	m_window      = other.m_window;
	m_windowTitle = other.m_windowTitle;
	m_resolution  = other.m_resolution;
	return *this;
}

Window& Window::operator=(Window&& other) noexcept
{
	if (this == &other)
		return *this;
	m_window      = other.m_window;
	m_windowTitle = std::move(other.m_windowTitle);
	m_resolution  = std::move(other.m_resolution);
	return *this;
}

void Window::swapBuffers() const
{
	glfwSwapBuffers(m_window);
}

void Window::setVsync(bool vsync_enabled)
{
	glfwSwapInterval(vsync_enabled);
}

GLFWwindow* Window::getGlfwWindow() const
{
	return m_window;
}

glm::ivec2 Window::getResolution() const
{
	return m_resolution;
}
