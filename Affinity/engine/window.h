#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>

class Window final
{
public:
	Window(const std::string& window_title, glm::ivec2 resolution);
	~Window();
	Window(const Window& other) = default;
	Window(Window&& other) noexcept;
	Window& operator=(const Window& other);
	Window& operator=(Window&& other) noexcept;

	/**
	 * \brief Swap window buffers
	 */
	void swapBuffers() const;

	/**
	 * \brief Set window vertical-sync
	 * \param vsync_enabled Whether or not vsync should be enabled
	 */
	void setVsync(bool vsync_enabled);

	GLFWwindow* getGlfwWindow() const;
	glm::ivec2 getResolution() const;
private:
	GLFWwindow* m_window = nullptr;

	std::string m_windowTitle;
	glm::ivec2 m_resolution;
};
