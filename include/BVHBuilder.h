#pragma once
#include <vector>
#include <fwd.hpp> //GLM
#include <functional>

struct Triangle;
struct Node;
class BVHBuilder
{
public:
	BVHBuilder();
	void build(std::vector<float> const& vertexRaw);
	void travel(glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT);
	void travelCycle(glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT);
	Node * const bvhToTexture();
	int getNodesSize();
	std::vector<Node> getNodes();
private:
	void buildRecurcive(int nodeIndex, std::vector<Triangle>const& vecTriangle);
	bool travelRecurcive(Node& node, glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT);
	bool travelStack(Node& node, glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT);
	int  texSize;
	std::vector<Node> nodeList;
	std::vector<Triangle> vecTriangle;
};

