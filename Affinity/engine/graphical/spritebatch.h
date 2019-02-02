#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <array>

struct Transform;
struct Sprite;

struct Vertex final
{
	glm::vec2 position;
	glm::vec2 uv;
	glm::vec4 color;

	friend bool operator==(const Vertex& lhs, const Vertex& rhs)
	{
		return std::tie(lhs.position, lhs.uv, lhs.color) == std::tie(rhs.position, rhs.uv, rhs.color);
	}
};

class Spritebatch final
{
public:
	Spritebatch();
	~Spritebatch();
	Spritebatch(const Spritebatch& other) = default;
	Spritebatch(Spritebatch&& other) noexcept = default;
	Spritebatch& operator=(const Spritebatch& other) = default;
	Spritebatch& operator=(Spritebatch&& other) noexcept = default;

	/**
	 * \brief Add a sprite to the spritebatch
	 * \param transform Transform component
	 * \param sprite Sprite component
	 */
	void addSprite(const Transform& transform, const Sprite& sprite);

	/**
	 * \brief Draw all sprites added since last clear
	 */
	void draw();

	/**
	 * \brief Remove all the sprites in the spritebatch
	 */
	void clear();

	size_t getNumSprites() const;
private:
	void loadSpritesIntoVertices();
	void addVertex(const Vertex& vertex);

	std::vector<std::pair<std::array<Vertex, 4>, double>> m_sprites;
	std::vector<Vertex> m_vertices;

	GLuint m_vao = 0;
	GLuint m_vbo = 0;

	uint32_t m_numSprites;
};
