#include "shader.h"

#include <fstream>

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path, std::vector<std::string> attributes)
	: m_attributes(std::move(attributes))
{
	m_programId = glCreateProgram();
	if (m_programId == 0)
	{
		throw std::runtime_error("Program could not be created");
	}

	auto vertSource = loadContentsFromFile(vertex_path);
	auto fragSource = loadContentsFromFile(fragment_path);

	const auto vertexShaderId   = glCreateShader(GL_VERTEX_SHADER);
	const auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	if (vertexShaderId == 0)
	{
		throw std::runtime_error("Vertex shader could not be created");
	}
	if (fragmentShaderId == 0)
	{
		throw std::runtime_error("Fragment shader could not be created");
	}

	generateShader(vertSource.c_str(), vertexShaderId);
	generateShader(fragSource.c_str(), fragmentShaderId);

	for (auto i = 0u; i < m_attributes.size(); ++i)
	{
		glBindAttribLocation(m_programId, i, m_attributes[i].c_str());
	}

	linkShaders(vertexShaderId, fragmentShaderId);
}

void Shader::bind() const
{
	glUseProgram(m_programId);
	for (auto i = 0u; i < m_attributes.size(); i++)
	{
		glEnableVertexAttribArray(i);
	}
}

void Shader::generateShader(const char* shader_source, const GLuint shader_id)
{
	glShaderSource(shader_id, 1, &shader_source, nullptr);
	glCompileShader(shader_id);

	GLint success = 0;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

	// Didn't compile successfully
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<char> errorLog(maxLength);
		glGetShaderInfoLog(shader_id, maxLength, &maxLength, &errorLog[0]);

		glDeleteShader(shader_id);

		throw std::runtime_error("Shader failed to compile:\n" + std::string(errorLog.begin(), errorLog.end()));
	}
}

void Shader::linkShaders(const GLuint vert_id, const GLuint frag_id) const
{
	glAttachShader(m_programId, vert_id);
	glAttachShader(m_programId, frag_id);

	glLinkProgram(m_programId);

	glDetachShader(m_programId, vert_id);
	glDetachShader(m_programId, frag_id);
	glDeleteShader(vert_id);
	glDeleteShader(frag_id);

	GLint isLinked = 0;
	glGetProgramiv(m_programId, GL_LINK_STATUS, &isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<char> errorLog(maxLength);
		glGetProgramInfoLog(m_programId, maxLength, &maxLength, &errorLog[0]);

		glDeleteProgram(m_programId);

		throw std::runtime_error("Shaders failed to link:\n" + std::string(errorLog.begin(), errorLog.end()));
	}
}

Shader::~Shader()
{
	glDeleteProgram(m_programId);
}

void Shader::setInt(const std::string& name, const int value) const
{
	glUniform1i(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, const float value) const
{
	glUniform1f(glGetUniformLocation(m_programId, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2 value) const
{
	glUniform2f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y);
}

void Shader::setVec3(const std::string& name, const glm::vec3 value) const
{
	glUniform3f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y, value.z);
}

void Shader::setVec4(const std::string& name, const glm::vec4 value) const
{
	glUniform4f(glGetUniformLocation(m_programId, name.c_str()), value.x, value.y, value.z, value.w);
}

void Shader::setMat3(const std::string& name, glm::mat3 mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat4(const std::string& name, glm::mat4 mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_programId, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

std::string Shader::loadContentsFromFile(const std::string& path)
{
	if (auto in = std::ifstream(path))
	{
		return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	}
	throw std::runtime_error("Could not load shader contents from file '" + path + "'");
}
