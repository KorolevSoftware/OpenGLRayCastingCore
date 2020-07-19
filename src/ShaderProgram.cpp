#include <fstream>
#include <array>
#include <iostream>
#include <tuple>
#include <glm.hpp>
#include "glad.h" // Opengl function
#include "Utils.h"
#include "ShaderProgram.h"

ShaderProgram::ShaderProgram(std::string const& vertexShaderPath, std::string const& fragmentShaderPath): programID(-1), texUnitSlotIndex(0)
{
	using std::make_tuple;
	using shader = std::tuple<std::string, int, unsigned int>; // <shader source code, shader type, shader id>
    
	std::array<shader, 2> shaderCode {
        make_tuple(loadShaderByFile(Utils::resourceDir + vertexShaderPath), GL_VERTEX_SHADER, 0),
        make_tuple(loadShaderByFile(Utils::resourceDir + fragmentShaderPath), GL_FRAGMENT_SHADER, 0) };

    // For check error
    int  success;
    char infoLog[512];
    for (shader& shaderItem : shaderCode)
    {
        auto& [sourceCode, shaderType, shaderID] = shaderItem;   
        shaderID = glCreateShader(shaderType);
        const char* sourceCodeData = sourceCode.data();
        glShaderSource(shaderID, 1, &sourceCodeData, NULL);
        glCompileShader(shaderID);
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
            std::cerr << "Error shader compilation failed:\n" << infoLog << std::endl;
        }
    }
    // Create program
    programID = glCreateProgram();

    for (shader& shaderItem : shaderCode)
        glAttachShader(programID, std::get<2>(shaderItem));

    glLinkProgram(programID);
    glGetProgramiv(programID, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "Error program compilation failed:\n" << infoLog << std::endl;
    }

    for (shader& shaderItem : shaderCode)
        glDeleteShader(std::get<2>(shaderItem));
}

std::string ShaderProgram::loadShaderByFile(std::string const& shaderPath)
{
	std::ifstream file(shaderPath);
    assert(file.is_open() && "Error open shader file");
	std::string shaderSource((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return shaderSource;
}

void ShaderProgram::bind()
{
    glUseProgram(programID);
    texUnitSlotIndex = 0;
}

void ShaderProgram::setTexture(std::string const& textureName, uint32_t texID, int texUnitSlot)
{
    glActiveTexture(GL_TEXTURE0 + texUnitSlot);
    glBindTexture(GL_TEXTURE_2D, texID);
    uint32_t textureLocation = glGetUniformLocation(programID, textureName.data());
    glUniform1i(textureLocation, texUnitSlot);
}

void ShaderProgram::setTextureAI(std::string const& textureName, TextureGL const& texture)
{
    glActiveTexture(GL_TEXTURE0 + texUnitSlotIndex);
    glBindTexture(GL_TEXTURE_2D, texture.textureID);
    uint32_t textureLocation = glGetUniformLocation(programID, textureName.data());
    glUniform1i(textureLocation, texUnitSlotIndex);

    texUnitSlotIndex++;
}

void ShaderProgram::setMatrix3x3(std::string const& matrixName, glm::mat3 const& matrix, int count, bool transpose)
{
    uint32_t matrixLocation = glGetUniformLocation(programID, matrixName.data());
    glUniformMatrix3fv(matrixLocation, count, transpose, &matrix[0][0]);
}

void ShaderProgram::setVec3(std::string const& vectorName, glm::vec3 const& vector)
{
    uint32_t vecLocation = glGetUniformLocation(programID, vectorName.data());
    glUniform3f(vecLocation, vector.x, vector.y, vector.z);
}

void ShaderProgram::setVec2(std::string const& vectorName, glm::vec2 const& vector)
{
    uint32_t vecLocation = glGetUniformLocation(programID, vectorName.data());
    glUniform2f(vecLocation, vector.x, vector.y);
}

void ShaderProgram::setInt(std::string const& intName, int data)
{
    uint32_t intLocation = glGetUniformLocation(programID, intName.data());
    glUniform1i(intLocation, data);
}

int ShaderProgram::getID()
{
    return programID;
}

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(programID);
}
