#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <vector>

class Shader final
{
public:
	Shader(const std::string& vertex_path, const std::string& fragment_path, std::vector<std::string> attributes);
	~Shader();
	Shader(const Shader& other) = default;
	Shader(Shader&& other) noexcept = default;
	Shader& operator=(const Shader& other) = default;
	Shader& operator=(Shader&& other) noexcept = default;

	/**
	 * \brief Bind the shader for use
	 */
	void bind() const;

	/*
	 * Setters for shader uniforms
	 */

	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec2(const std::string& name, glm::vec2 value) const;
	void setVec3(const std::string& name, glm::vec3 value) const;
	void setVec4(const std::string& name, glm::vec4 value) const;
	void setMat3(const std::string& name, glm::mat3 mat) const;
	void setMat4(const std::string& name, glm::mat4 mat) const;
private:
	void generateShader(const char* shader_source, GLuint shader_id);
	void linkShaders(GLuint vert_id, GLuint frag_id) const;

	static std::string loadContentsFromFile(const std::string& path);

	GLuint m_programId = 0;
	std::vector<std::string> m_attributes;
};
