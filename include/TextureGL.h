#pragma once
#include <cstdint>

enum class TextureGLType
{
	VertexDataXYZ
};

class TextureGL 
{
public:
	TextureGL(int width, int height, TextureGLType datatype, const void* data);
	TextureGL(TextureGL&& other);
	int getWidth();
	int getHeight();
	void bind();
	~TextureGL();
	friend class ShaderProgram;

private:
	int width;
	int height;
	void VertexDataXYZToTexture(int width, int height, const void* data);
	uint32_t textureID;
};