#include "engine.h"
#include "window.h"
#include "ecs/components.h"
#include "ecs/systems/boat_system.h"
#include "ecs/systems/script_system.h"
#include "ecs/systems/physics_system.h"
#include "ecs/systems/particle_system.h"
#include "ecs/systems/sprite_render_system.h"

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

static plog::ColorConsoleAppender<plog::TxtFormatter> COLOR_CONSOLE_APPENDER;

bool Engine::init()
{
	if (m_initialized) { return true; }
	m_initialized = true;

	try
	{
		initLogger();
		LOG_INFO << "Initializing engine";
		initGlfw();
		initWindow();
		initGraphics();
		initEcs();
		initTimers();
	}
	catch (const std::runtime_error& err)
	{
		LOG_FATAL << err.what();
		return false;
	}

	LOG_INFO << "Engine initialized";
	return true;
}

int Engine::run()
{
	// Engine must be initialized before it is run
	if (!m_initialized)
	{
		LOG_FATAL << "Cannot run uninitialized engine";
		return 2;
	}

	// Run main game loop
	loop();

	// Shutdown engine
	shutdown();
	return 0;
}

void Engine::stop()
{
	m_running = false;
}

Window& Engine::getWindow() const
{
	return *m_window;
}

Input& Engine::getInput() const
{
	return *m_input;
}

Renderer& Engine::getRenderer() const
{
	return *m_renderer;
}

EntityComponentSystem& Engine::getEntityComponentSystem() const
{
	return *m_ecs;
}

Timer& Engine::getFrameTimer() const
{
	return *m_frameTimer;
}

void Engine::loop()
{
	while (isRunning())
	{
		// Update
		m_input->update();
		m_frameTimer->update();

		m_ecs->update(*this);

		// Render
		glClear(GL_COLOR_BUFFER_BIT);

		m_renderer->render();

		// Update window
		m_window->swapBuffers();
	}
}

void Engine::shutdown()
{
	LOG_INFO << "Shutting down engine";
}

void Engine::initLogger() const
{
	plog::init(plog::verbose, &COLOR_CONSOLE_APPENDER);
}

void Engine::initGlfw()
{
	LOG_VERBOSE << "Initializing GLFW";

	glfwSetErrorCallback([](int error, const char* description)
	{
		LOG_ERROR << "[GLFW][" << error << "] " << description;
	});

	if (!glfwInit())
	{
		throw std::runtime_error("GLFW failed to initialize");
	}
}

void Engine::initWindow()
{
	LOG_VERBOSE << "Initializing window";
	m_window = std::make_unique<Window>("Affinity", glm::ivec2(1280, 720));
	glfwSetWindowUserPointer(m_window->getGlfwWindow(), this);

	LOG_VERBOSE << "Initializing input";
	m_input = std::make_unique<Input>(m_window->getGlfwWindow());
}

void Engine::initGraphics()
{
	LOG_VERBOSE << "Initializing GLEW";
	if (const auto err = glewInit())
	{
		throw std::runtime_error(
			"GLEW failed to initialize: " + std::string(reinterpret_cast<const char*>(glewGetErrorString(err))));
	}

	LOG_VERBOSE << "Initializing renderer";
	m_renderer = std::make_unique<Renderer>(*this);
}

void Engine::initEcs()
{
	LOG_VERBOSE << "Initializing ECS";
	m_ecs = std::make_unique<EntityComponentSystem>();

	// Register components
	m_ecs->registerComponent<Transform>();
	m_ecs->registerComponent<Sprite>();
	m_ecs->registerComponent<Camera>();
	m_ecs->registerComponent<Script>();
	m_ecs->registerComponent<Boat>();
	m_ecs->registerComponent<BoatAi>();
	m_ecs->registerComponent<Cannonball>();
	m_ecs->registerComponent<Particle>();

	// Add systems
	m_ecs->addSystem<BoatSystem>();
	m_ecs->addSystem<ScriptSystem>();
	m_ecs->addSystem<PhysicsSystem>();
	m_ecs->addSystem<ParticleSystem>();
	m_ecs->addSystem<SpriteRenderSystem>();
}

void Engine::initTimers()
{
	m_frameTimer = std::make_unique<Timer>();
}

bool Engine::isRunning()
{
	if (glfwWindowShouldClose(m_window->getGlfwWindow()))
	{
		m_running = false;
	}
	return m_running;
}
