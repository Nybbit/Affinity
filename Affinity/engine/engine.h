#pragma once
#include "window.h"
#include "input.h"
#include "graphical/renderer.h"
#include "ecs/ecs.h"
#include "util/timer.h"

#include <memory>

class Engine final
{
public:
	Engine() = default;

	/**
	 * \brief Initializes the game engine
	 * \return Whether or not initialization was successful
	 */
	bool init();

	/**
	 * \brief Starts and runs the engine
	 * \return Exit code
	 */
	int run();

	/**
	 * \brief Stop then engine from running
	 */
	void stop();

	Window& getWindow() const;
	Input& getInput() const;
	Renderer& getRenderer() const;
	EntityComponentSystem& getEntityComponentSystem() const;
	Timer& getFrameTimer() const;
private:
	/**
	 * \brief Main loop
	 */
	void loop();

	/**
	 * \brief Shutdown the engine
	 */
	void shutdown();

	// Initialization functions
	void initLogger() const;
	void initGlfw();
	void initWindow();
	void initGraphics();
	void initEcs();
	void initTimers();

	bool isRunning();

	bool m_initialized = false;
	bool m_running     = true;

	std::unique_ptr<Window> m_window             = nullptr;
	std::unique_ptr<Input> m_input               = nullptr;
	std::unique_ptr<Renderer> m_renderer         = nullptr;
	std::unique_ptr<EntityComponentSystem> m_ecs = nullptr;

	std::unique_ptr<Timer> m_frameTimer = nullptr;
};
