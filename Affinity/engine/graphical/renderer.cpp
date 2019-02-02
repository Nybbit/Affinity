#include "renderer.h"
#include "../engine.h"
#include "../ecs/components.h"

#include "../../imgui/imgui.h"
#include "../../imgui/examples/imgui_impl_opengl3.h"
#include "../../imgui/examples/imgui_impl_glfw.h"
#include "../../imgui/imgui_internal.h"

#include <plog/Log.h>

Renderer::Renderer(Engine& engine)
	: m_engine(engine),
	  m_spritesheet("data/textures"),
	  m_worldShader("data/shaders/world.vert", "data/shaders/world.frag", std::vector<std::string>{"inPosition"}),
	  m_spriteShader("data/shaders/sprite.vert", "data/shaders/sprite.frag",
	                 std::vector<std::string>({"inPosition", "inUv", "inColor"}))
{
	// Alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Bind spritesheet texture to texture0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_spritesheet.getTextureId());

	m_spriteShader.bind();
	m_spriteShader.setInt("tex", 0);

	m_worldShader.bind();
	m_worldShader.setInt("tex", 0);

	initGui();
}

Renderer::~Renderer()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::render()
{
	m_worldShader.bind();

	m_worldShader.setVec4("uv", m_spritesheet.getUv("water"));
	m_worldShader.setVec2("cameraPos", m_activeCamera.getComponent<Transform>()->position);
	m_worldShader.setVec2("cameraScale", m_activeCamera.getComponent<Transform>()->scale);
	m_worldShader.setVec2("screenResolution", m_engine.getWindow().getResolution());

	// Draw fullscreen quad
	{
		static const std::array<glm::vec2, 4> VERTICES = { glm::vec2(-1.0, -1.0), glm::vec2(1.0, -1.0), glm::vec2(-1.0, 1.0), glm::vec2(1.0, 1.0) };
		static GLuint vao = 0;
		static GLuint vbo = 0;
		if (vao == 0)
		{
			glGenVertexArrays(1, &vao);
			glBindVertexArray(vao);

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, VERTICES.size() * sizeof(VERTICES[0]), VERTICES.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICES[0]), nullptr);
		}
		else
		{
			glBindVertexArray(vao);
		}

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glBindVertexArray(0);
	}

	m_spriteShader.bind();

	if (m_activeCamera.isValid())
	{
		const auto ortho = m_activeCamera.getComponent<Camera>()->ortho;

		const auto orthoMatrix = glm::ortho(0.0f, ortho.x, 0.0f, ortho.y);

		const auto pos   = m_activeCamera.getComponent<Transform>()->position;
		const auto scale = m_activeCamera.getComponent<Transform>()->scale;

		m_spriteShader.setMat4("cameraMatrix",
		                       glm::translate(
			                       glm::scale(orthoMatrix, glm::vec3(scale, 1.0f)),
			                       -glm::vec3(pos - ortho / 2.0f / scale, 0.0f)
		                       ));

		m_spritebatch.draw();
	}
	else
	{
		// todo: find alternative to spamming "No active camera"
		LOG_WARNING << "No active camera";
	}

	drawGui();

	m_spritebatch.clear();
}

void Renderer::setCamera(Entity entity)
{
	if (!entity.isValid()
	    || !entity.getComponent<Transform>()
	    || !entity.getComponent<Camera>())
	{
		LOG_WARNING << "Invalid camera";
		return;
	}

	m_activeCamera = entity;
}

Entity Renderer::getCamera()
{
	return m_activeCamera;
}

Spritesheet& Renderer::getSpritesheet()
{
	return m_spritesheet;
}

Spritebatch& Renderer::getSpritebatch()
{
	return m_spritebatch;
}

void Renderer::initGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_engine.getWindow().getGlfwWindow(), true);
	ImGui_ImplOpenGL3_Init();
}

void Renderer::drawGui()
{
	// Setup
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("debug");
	ImGui::Text("FPS:       %d", m_engine.getFrameTimer().getUpdatesPerSecond());
	ImGui::Text("Frametime: %fms", 1000.0f / m_engine.getFrameTimer().getUpdatesPerSecond());
	ImGui::Text("Entities:  %d/%d", m_engine.getEntityComponentSystem().getNumActiveEntities(),
	            m_engine.getEntityComponentSystem().getNumEntities());
	ImGui::Text("Sprites:   %d", m_spritebatch.getNumSprites());
	ImGui::End();

	// Draw it
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
