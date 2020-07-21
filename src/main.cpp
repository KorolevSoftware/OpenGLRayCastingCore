#include <assert.h>
#include <gtc/matrix_transform.hpp>
#include <fwd.hpp> //GLM
#include <iostream>
#include <map>
#include "Utils.h"
#include "ModelLoader.h"
#include "glad.h" // Opengl function loader
#include "BVHBuilder.h"
#include "TextureGL.h"
#include "ShaderProgram.h"
#include "SDLHelper.h"


using std::vector;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;


constexpr int WinWidth = 1920;
constexpr int WinHeight = 1080;


std::map<int, bool> buttinInputKeys; //keyboard key
float yaw = 0.0f;          // for cam rotate
float pitch = 0.0f;        // for cam rotate


TextureGL BVHNodesToTexture(BVHBuilder& bvh)
{
	float const* texNodeData = (float*)(bvh.bvhToTexture());
	int texWidthNode = bvh.getNodesSize();
	return TextureGL(texWidthNode, texWidthNode, TextureGLType::VertexDataXYZ, texNodeData);
}


TextureGL loadGeometry(BVHBuilder& bvh, std::string const& path)
{
	vector<float> vertex;
	vector<float> normal;
	vector<float> uv;

	ModelLoader::Obj(path, vertex, normal, uv);
	bvh.build(vertex);

	uint32_t vertexCount = vertex.size() / 3; // 3 vertex component x,y,z
	int sqrtVertexCount = ceil(sqrt(vertexCount)); // for sqrt demension 
	int texWidthPos = Utils::powerOfTwo(sqrtVertexCount); // texture demension sqrt
	vertex.resize(texWidthPos * texWidthPos * 3, 0.0); // for pack x,y,z to  r,g,b

	return TextureGL(texWidthPos, texWidthPos, TextureGLType::VertexDataXYZ, vertex.data());
}


// FPS Camera rotate
void updateMatrix(glm::mat3& viewToWorld)
{
	mat4 matPitch = mat4(1.0);
	mat4 matYaw = mat4(1.0);

	matPitch = glm::rotate(matPitch, glm::radians(pitch), vec3(1, 0, 0));
	matYaw = glm::rotate(matYaw, glm::radians(yaw), vec3(0, 1, 0));

	mat4 rotateMatrix = matYaw * matPitch;
	viewToWorld = mat3(rotateMatrix);
}


// FPS Camera move
void cameraMove(vec3& location, glm::mat3 const& viewToWorld)
{
	float speed = 1;
	if (buttinInputKeys[SDLK_w])
		location += viewToWorld * vec3(0, 0, 1) * speed;

	if (buttinInputKeys[SDLK_s])
		location += viewToWorld * vec3(0, 0, -1) * speed;

	if (buttinInputKeys[SDLK_a])
		location += viewToWorld * vec3(-1, 0, 0) * speed;

	if (buttinInputKeys[SDLK_d])
		location += viewToWorld * vec3(1, 0, 0) * speed;

	if (buttinInputKeys[SDLK_q])
		location += viewToWorld * vec3(0, 1, 0) * speed;

	if (buttinInputKeys[SDLK_e])
		location += viewToWorld * vec3(0, -1, 0) * speed;
}


int main(int ArgCount, char** Args)
{
	// Set Opengl Specification
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// SDL window and Opengl
	SDLWindowPtr window(SDL_CreateWindow("OpenGL ray casting", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WinWidth, WinHeight, SDL_WINDOW_OPENGL));
	if (!window) {
		std::cerr << "Failed create window" << std::endl;
		return -1;
	}

	SDLGLContextPtr context(SDL_GL_CreateContext(window));
	if (!context) {
		std::cerr << "Failed to create OpenGL context" << std::endl;
		return -1;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "OpenGL version " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	// GL 3.3, you must have though 1 VAO for correct work vertex shader
	uint32_t VAO;
	glGenVertexArrays(1, &VAO);

	// Load geometry, build BVH && and load data to texture
	BVHBuilder* bvh = new BVHBuilder(); // Big object
	TextureGL texPos = loadGeometry(*bvh, "models/BullPlane.obj");
	TextureGL texNode = BVHNodesToTexture(*bvh);
	ShaderProgram shaderProgram("shaders/vertex.vert", "shaders/raytracing.frag");

	// Variable for camera  
	vec3 location = vec3(0, 0.1, -20);
	mat3 viewToWorld = mat3(1.0f);

	// Add value  in map
	buttinInputKeys[SDLK_w] = false;
	buttinInputKeys[SDLK_s] = false;
	buttinInputKeys[SDLK_a] = false;
	buttinInputKeys[SDLK_d] = false;
	buttinInputKeys[SDLK_q] = false;
	buttinInputKeys[SDLK_e] = false;

	// Event loop
	SDL_Event Event;
	auto keyIsInside = [&Event] {return buttinInputKeys.count(Event.key.keysym.sym); }; // check key inside in buttinInputKeys

	while (true)
	{
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
				return 0;

			if (Event.type == SDL_MOUSEMOTION) {
				pitch += Event.motion.yrel * 0.03;
				yaw += Event.motion.xrel * 0.03;
			}

			if (Event.type == SDL_KEYDOWN && keyIsInside())
				buttinInputKeys[Event.key.keysym.sym] = true;

			if (Event.type == SDL_KEYUP && keyIsInside())
				buttinInputKeys[Event.key.keysym.sym] = false;

			if (Event.type == SDL_KEYDOWN && Event.key.keysym.sym == SDLK_ESCAPE)
				return 0;
		}

		cameraMove(location, viewToWorld);
		updateMatrix(viewToWorld);

		// Render/Draw
		// Clear the colorbuffer
		glViewport(0, 0, WinWidth, WinHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		shaderProgram.bind();
		glBindVertexArray(VAO);
		// Set shader variable
		shaderProgram.setTextureAI("texPosition", texPos);
		shaderProgram.setTextureAI("texNode", texNode);
		shaderProgram.setMatrix3x3("viewToWorld", viewToWorld);
		shaderProgram.setVec3("location", location);
		shaderProgram.setVec2("screeResolution", vec2(WinWidth, WinHeight));
		shaderProgram.setInt("bvhWidth", texNode.getWidth());
		shaderProgram.setInt("texPosWidth", texPos.getWidth());
		// Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		SDL_GL_SwapWindow(window);
	}
}
