#pragma once
#include <string>
#include <fwd.hpp>
#include "TextureGL.h"
class ShaderProgram 
{
public:
	ShaderProgram(std::string const& vertexShaderPath, std::string const& fragmentShaderPath);
	void bind();
	void setTexture(std::string const& textureName, uint32_t texID, int texUnitSlot);
	void setTextureAI(std::string const& textureName, TextureGL const& texture);
	void setMatrix3x3(std::string const& matrixName, glm::mat3 const& matrix, int count = 1, bool transpose = false);
	void setVec3(std::string const& vectorName, glm::vec3 const& vector);
	void setVec2(std::string const& vectorName, glm::vec2 const& vector);
	void setInt(std::string const& vectorName, int data);
	int getID();
	~ShaderProgram();

private:
	std::string loadShaderByFile(std::string const& shaderPath);
	uint32_t programID;
	int texUnitSlotIndex;
};