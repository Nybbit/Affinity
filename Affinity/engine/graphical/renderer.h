#pragma once
#include "spritesheet.h"
#include "shader.h"
#include "spritebatch.h"

#include "../ecs/ecs.h"
#include "../ecs/entity.h"

class Renderer final
{
public:
	Renderer(Engine& engine);
	~Renderer();
	Renderer(const Renderer& other) = delete;
	Renderer(Renderer&& other) noexcept = delete;
	Renderer& operator=(const Renderer& other) = delete;
	Renderer& operator=(Renderer&& other) noexcept = delete;

	/**
	 * \brief Draw spritebatches with their shaders
	 */
	void render();

	void setCamera(Entity entity);
	Entity getCamera();

	Spritesheet& getSpritesheet();
	Spritebatch& getSpritebatch();
private:
	void initGui();
	void drawGui();

	Engine& m_engine;

	Spritesheet m_spritesheet;

	Shader m_worldShader;
	Shader m_spriteShader;
	Spritebatch m_spritebatch;

	Entity m_activeCamera;
};
