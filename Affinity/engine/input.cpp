#include "input.h"
#include "engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <plog/Log.h>

Input::Input(GLFWwindow* window)
{
	// Key input
	glfwSetKeyCallback(window,
		[](GLFWwindow* win, int key, int /*scancode*/, int action, int /*mods*/)
	{
		const auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(win));

		if (action == GLFW_PRESS)
		{
			engine->getInput().pressKeyDown(key);
		}
		else if (action == GLFW_RELEASE)
		{
			engine->getInput().pressKeyUp(key);
		}
	});
}

bool Input::isKeyDown(const unsigned key) const
{
	const auto it = m_keyStates.find(key);
	return it != m_keyStates.end() && (it->second == KeyStateEnum::PRESSED || it->second == KeyStateEnum::DOWN);
}

bool Input::isKeyPressed(const unsigned key) const
{
	const auto it = m_keyStates.find(key);
	return it != m_keyStates.end() && (it->second == KeyStateEnum::PRESSED);
}

void Input::update()
{
	// Update "single-update" states (pressed and released)
	for (auto& k : m_keyStates)
	{
		if (k.second == KeyStateEnum::PRESSED)
		{
			k.second = KeyStateEnum::DOWN;
		}
		else if (k.second == KeyStateEnum::RELEASED)
		{
			k.second = KeyStateEnum::UP;
		}
	}

	// Poll GLFW events
	glfwPollEvents();
}

void Input::pressKeyDown(unsigned key)
{
	const auto it = m_keyStates.find(key);
	if (it == m_keyStates.end())
	{
		m_keyStates.emplace(key, KeyStateEnum::PRESSED);
	}
	else if (it->second == KeyStateEnum::UP)
	{
		m_keyStates[key] = KeyStateEnum::PRESSED;
	}
}

void Input::pressKeyUp(unsigned key)
{
	const auto it = m_keyStates.find(key);
	if (it == m_keyStates.end())
	{
		m_keyStates.emplace(key, KeyStateEnum::RELEASED);
	}
	else if (it->second == KeyStateEnum::PRESSED || it->second == KeyStateEnum::DOWN)
	{
		m_keyStates[key] = KeyStateEnum::RELEASED;
	}
}
