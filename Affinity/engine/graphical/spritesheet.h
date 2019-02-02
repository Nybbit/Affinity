#pragma once
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <string>
#include <experimental/filesystem>
#include <unordered_map>
#include <array>

enum ImageTypeFlags
{
	PNG = 0x01,
	JPEG = 0x02,
	BMP = 0x04
};

struct TextureData final
{
	TextureData(const std::string& path, const std::string& texture_name, const glm::ivec2 dimensions,
	            const int channels)
		: path(path), textureName(texture_name), dimensions(dimensions), channels(channels)
	{
	}

	std::string path;
	std::string textureName;
	glm::ivec2 dimensions;
	glm::ivec2 position = glm::ivec2(-1);
	int channels;

	friend bool operator<(const TextureData& lhs, const TextureData& rhs)
	{
		const auto aSize = std::max(lhs.dimensions.x, lhs.dimensions.y);
		const auto bSize = std::max(rhs.dimensions.x, rhs.dimensions.y);
		return aSize > bSize;
	}

	friend bool operator==(const TextureData& lhs, const TextureData& rhs)
	{
		return std::tie(lhs.textureName, lhs.dimensions, lhs.channels) == std::tie(
			       rhs.textureName, rhs.dimensions, rhs.channels);
	}
};

struct Node final
{
	enum FitTypeEnum
	{
		DOES_NOT_FIT,
		PERFECT_FIT,
		EXTRA_SPACE
	};

	FitTypeEnum fits(glm::ivec2 dimensions) const;
	Node* insert(TextureData& data);

	std::array<std::unique_ptr<Node>, 2> children = {nullptr, nullptr};
	glm::ivec4 rectangle                          = glm::ivec4(0);
	TextureData* textureData                      = nullptr;
};

class Spritesheet final
{
public:
	/**
	 * \brief Create a spritesheet for importing
	 */
	explicit Spritesheet();
	/**
	 * \brief Generate a spritesheet
	 * \param directory Directory to search for textures in
	 * \param image_type_flags Types of images to search for, bitflag
	 */
	explicit Spritesheet(const std::string& directory, unsigned image_type_flags = PNG);
	~Spritesheet();
	Spritesheet(const Spritesheet& other) = delete;
	Spritesheet(Spritesheet&& other) noexcept = delete;
	Spritesheet& operator=(const Spritesheet& other) = delete;
	Spritesheet& operator=(Spritesheet&& other) noexcept = delete;

	/**
	 * \brief Get the uv for a texture in the spritesheet
	 * \param texture_name Name of texture you want the UV of
	 * \return UV of texture
	 */
	glm::vec4 getUv(const std::string& texture_name);
	GLuint getTextureId() const;

	/**
	 * \brief Export spritesheet and spritesheet data for quick importing
	 * \param directory Directory to export spritesheet to
	 */
	void exportSpritesheet(const std::string& directory);
	/**
	 * \brief Import a premade spritesheet
	 * \param directory Directory to import spritesheet from
	 */
	void importSpritesheet(const std::string& directory);
private:
	/**
	 * \brief Generate the spritesheet
	 */
	void generate();
	/**
	 * \brief Adds texture data to spritesheet
	 * \param p Path to texture
	 */
	void addTexture(const std::experimental::filesystem::directory_entry& p);
	/**
	 * \brief Arranges textures into a square
	 */
	void packTextures();
	/**
	 * \brief Turns spritesheet into a texture for OpenGL
	 */
	void generateOpenGlTexture();
	/**
	 * \brief Cleans up anything that isn't required for use after generation
	 */
	void cleanup();

	std::string m_directory;
	unsigned m_imageTypeFlags;
	std::vector<TextureData> m_unprocessedTextures;
	std::unique_ptr<Node> m_root = nullptr;

	bool m_initialized                 = false;
	glm::ivec2 m_spritesheetDimensions = glm::ivec2(0);
	std::vector<uint8_t> m_pixels;
	std::unordered_map<std::string, glm::vec4> m_elements;
	GLuint m_textureId = 0;
	bool m_hasDefault  = false;
};
