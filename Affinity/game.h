#pragma once
#include "engine/engine.h"

class Game final
{
public:
	Game() = default;

	/**
	 * \brief Run the game
	 * \return Exit code
	 */
	int run();
private:
	void init();

	Engine m_engine;
};
