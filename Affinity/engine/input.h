#pragma once
#include "Window.h"

#include <unordered_map>

class Input final
{
public:
	explicit Input(GLFWwindow* window);

	/**
	 * \brief Query whether a key is being held down
	 * \param key SDL Key
	 * \return Whether or not the key is down
	 */
	bool isKeyDown(unsigned key) const;

	/**
	 * \brief Query whether a key was just pressed
	 * \param key SDL Key
	 * \return Whether or not the key was just pressed
	 */
	bool isKeyPressed(unsigned key) const;
private:
	friend class Engine;

	enum class KeyStateEnum
	{
		UP,
		PRESSED,
		DOWN,
		RELEASED
	};

	/**
	 * \brief Update keystates
	 */
	void update();

	/**
	 * \brief Mark key as being pressed down
	 * \param key SDL key
	 */
	void pressKeyDown(unsigned key);

	/**
	 * \brief Mark key as being let go
	 * \param key SDL Key
	 */
	void pressKeyUp(unsigned key);

	std::unordered_map<unsigned, KeyStateEnum> m_keyStates;
};
