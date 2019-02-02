#include "spritebatch.h"
#include "../ecs/components.h"

#include <glm/ext/matrix_transform.hpp>
#include <plog/Log.h>

#include <algorithm>
#include <chrono>

Spritebatch::Spritebatch()
{
	// generate vao
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	// generate vbo
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      reinterpret_cast<void*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      reinterpret_cast<void*>(offsetof(Vertex, uv)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      reinterpret_cast<void*>(offsetof(Vertex, color)));

	glBindVertexArray(0);
}

Spritebatch::~Spritebatch()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo);
}

void Spritebatch::addSprite(const Transform& transform, const Sprite& sprite)
{
	auto modelMatrix = glm::mat4(1);
	modelMatrix      = glm::translate(modelMatrix, glm::vec3(transform.position, 0.0f));

	const auto scalePoint  = glm::vec2(0.5f);
	const auto rotatePoint = scalePoint;

	modelMatrix = glm::translate(modelMatrix, glm::vec3(rotatePoint, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, transform.rotation, glm::vec3(0.0f, 0.0f, 1.0f));
	modelMatrix = glm::translate(modelMatrix, -glm::vec3(rotatePoint, 0.0f));

	modelMatrix = glm::translate(modelMatrix, glm::vec3(scalePoint, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(transform.scale, 1.0f));
	modelMatrix = glm::translate(modelMatrix, -glm::vec3(scalePoint, 0.0f));

	const auto bl = Vertex({
		glm::vec2(modelMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
		glm::vec2(sprite.uv.x, sprite.uv.y),
		sprite.color
	});
	const auto br = Vertex({
		glm::vec2(modelMatrix * glm::vec4(1, 0.0f, 0.0f, 1.0f)),
		glm::vec2(sprite.uv.x + sprite.uv.z, sprite.uv.y),
		sprite.color
	});
	const auto tr = Vertex({
		glm::vec2(modelMatrix * glm::vec4(1, 1, 0.0f, 1.0f)),
		glm::vec2(sprite.uv.x + sprite.uv.z, sprite.uv.y + sprite.uv.w),
		sprite.color
	});
	const auto tl = Vertex({
		glm::vec2(modelMatrix * glm::vec4(0.0f, 1, 0.0f, 1.0f)),
		glm::vec2(sprite.uv.x, sprite.uv.y + sprite.uv.w),
		sprite.color
	});

	m_sprites.emplace_back(std::array<Vertex, 4>({bl, br, tr, tl}), sprite.depth);
}

void Spritebatch::draw()
{
	loadSpritesIntoVertices();
	glBindVertexArray(m_vao);

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data(), GL_DYNAMIC_DRAW);

	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

	glBindVertexArray(0);
}

void Spritebatch::clear()
{
	m_vertices.clear();
	m_sprites.clear();
}

size_t Spritebatch::getNumSprites() const
{
	return m_sprites.size();
}

void Spritebatch::loadSpritesIntoVertices()
{
	if (!m_vertices.empty()) return;

	// This sort takes up ~45% of the function
	std::stable_sort(m_sprites.begin(), m_sprites.end(),
	                 [](auto& lhs, auto& rhs)
	                 {
		                 return lhs.second > rhs.second;
	                 });

	for (auto& s : m_sprites)
	{
		addVertex(s.first[0]);
		addVertex(s.first[1]);
		addVertex(s.first[2]);
		addVertex(s.first[2]);
		addVertex(s.first[3]);
		addVertex(s.first[0]);
	}
}

void Spritebatch::addVertex(const Vertex& vertex)
{
	/*
	 * It's faster to just add the vertex and not check for repeated vertices
	 * It's a lot more useful in 3D development when vertices are much more
	 * likely to be shared
	 */
	m_vertices.push_back(vertex);
}
