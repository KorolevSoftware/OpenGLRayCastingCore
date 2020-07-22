#include <glm.hpp>
#include <algorithm>
#include <stack>
#include <Utils.h>
#include "BVHBuilder.h"
using glm::vec3;

vec3 minVecComponent(vec3 const& a, vec3 const& b)
{
	return { std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) };
}

vec3 maxVecComponent(vec3 const& a, vec3 const& b)
{
	return { std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) };
}

struct AABB
{
private:
	vec3 min;
	vec3 max;

public:
	AABB():
		min({ 0, 0, 0 }),
		max({ 0, 0, 0 }) {}

	AABB(vec3 min, vec3 max) :
		min(min),
		max(max) {}


	void surrounding(AABB const& aabb)
	{
		min = minVecComponent(min, aabb.min);
		max = maxVecComponent(max, aabb.max);
	}

	bool rayIntersect(vec3& origin, vec3& direction, float& mint)
	{
		if (glm::all(glm::greaterThan(origin, this->min)) && glm::all(glm::lessThan(origin, this->max)))
			return true;

		vec3 t0 = (this->min - origin) / direction;
		vec3 t1 = (this->max - origin) / direction;
		vec3 tmin = glm::min(t0, t1), tmax = glm::max(t0, t1);
		float tminf = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
		float tmaxf = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

		if (tminf > tmaxf)
			return false;

	
		return tminf>0.0f;
	}

	vec3& getMin() { return min; }
	vec3& getMax() { return max; }
};

struct Node
{
	/*bool leftChildIsTriangle;
	bool rightChildIsTriangle;*/
	float childIsTriangle;
	float leftChild;
	float rightChild;
	AABB aabb;

	Node() : /*leftChildIsTriangle(false), rightChildIsTriangle(false)*/ childIsTriangle(0), leftChild(-1), rightChild(-1) {}
};

struct Triangle
{
private:
	vec3 vertex1;
	vec3 vertex2;
	vec3 vertex3;
	int index;
	vec3 center;
	AABB aabb;

public:
	Triangle(vec3 vertex1, vec3 vertex2, vec3 vertex3, int index) :
		vertex1(vertex1),
		vertex2(vertex2),
		vertex3(vertex3),
		index(index)
	{
		aabb = genAABB();
		center = genCenter();
	}

	Triangle() :Triangle(vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), -1) {};

	bool rayIntersect(vec3& origin, vec3& direction, vec3& normal, float& mint)
	{
		vec3 e1 = vertex2 - vertex1;
		vec3 e2 = vertex3 - vertex1;
		vec3 P = glm::cross(direction, e2);
		float det = glm::dot(e1, P);

		if (abs(det) < 1e-4)
			return false;

		float inv_det = 1.0 / det;
		vec3 T = origin - vertex1;
		float u = glm::dot(T, P) * inv_det;

		if (u < 0.0 || u > 1.0)
			return false;

		vec3 Q = glm::cross(T, e1);
		float v = glm::dot(direction, Q) * inv_det;

		if (v < 0.0 || (v + u) > 1.0)
			return false;

		float tt = glm::dot(e2, Q) * inv_det;

		if (tt <= 0.0 || tt > mint)
			return false;

		normal = glm::normalize(glm::cross(e1, e2));
		mint = tt;
		return true;
	}

	vec3& getCenter() { return center; }
	AABB& getAABB() { return aabb; }
	int getIndex() { return index; }

	vec3 getCenter() const { return center; }
	AABB getAABB() const { return aabb; }
	int getIndex() const { return index; }

private:
	vec3 genCenter()
	{
		vec3 sum = vertex1 + vertex2 + vertex3;
		vec3 centerDim = sum / 3.0f;
		return centerDim;
	}

	AABB genAABB()
	{
		return AABB(
			minVecComponent(minVecComponent(vertex1, vertex2), vertex3),
			maxVecComponent(maxVecComponent(vertex1, vertex2), vertex3));
	}
};

BVHBuilder::BVHBuilder() {}

void BVHBuilder::build(std::vector<float> const& vertexRaw)
{
	size_t nodeSize = sizeof(Node);
	nodeList.push_back(Node());

	int floatInTriangle = 9; // x,y,z x,y,z x,y,z = 9 float
	for (int index = 0; index < vertexRaw.size(); index += floatInTriangle)
	{
		vecTriangle.emplace_back(
			vec3(vertexRaw[index + 0], vertexRaw[index + 1], vertexRaw[index + 2]),
			vec3(vertexRaw[index + 3], vertexRaw[index + 4], vertexRaw[index + 5]),
			vec3(vertexRaw[index + 6], vertexRaw[index + 7], vertexRaw[index + 8]),
			index / floatInTriangle);
	}
	nodeList.reserve(vecTriangle.size());
	buildRecurcive(0, vecTriangle);
}

void BVHBuilder::travel(glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT)
{
	travelRecurcive(nodeList[0], origin, direction, color, minT);
}

void BVHBuilder::travelCycle(glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT)
{
	travelStack(nodeList[0], origin, direction, color, minT);
}

Node *const BVHBuilder::bvhToTexture()
{
	int vertexCount = nodeList.size() * 3;
	int sqrtVertCount = ceil(sqrt(vertexCount));
	texSize = Utils::powerOfTwo(sqrtVertCount);
	nodeList.resize(texSize * texSize);

	return nodeList.data();
}

int BVHBuilder::getNodesSize()
{
	return texSize;
}

std::vector<Node> BVHBuilder::getNodes()
{
	return nodeList;
}



void BVHBuilder::buildRecurcive(int nodeIndex, std::vector<Triangle> const& vecTriangle)
{
	//Build Bpun box for triangles in vecTriangle
	AABB tempaabb = vecTriangle[0].getAABB();
	for (Triangle const& tri : vecTriangle)
		tempaabb.surrounding(tri.getAABB());

	Node& node = nodeList[nodeIndex];
	node.aabb = tempaabb;

	if (vecTriangle.size() == 2)
	{
		/*node.rightChildIsTriangle = true;
		node.leftChildIsTriangle = true;*/
		node.childIsTriangle = 3;
		node.leftChild = vecTriangle[0].getIndex();
		node.rightChild = vecTriangle[1].getIndex();
		return;
	}

	// seach max dimenson for split 
	vec3 maxVec = vecTriangle[0].getCenter();
	vec3 minVec = vecTriangle[0].getCenter();
	vec3 centerSum(0, 0, 0);

	for (Triangle const& tri : vecTriangle)
	{
		maxVec = maxVecComponent(tri.getCenter(), maxVec);
		minVec = minVecComponent(tri.getCenter(), minVec);
		centerSum += tri.getCenter();
	}
	vec3 midPoint = centerSum / (float)vecTriangle.size();
	vec3 len = glm::abs(maxVec - minVec);

	int axis = 0;

	if (len.y > len.x)
		axis = 1;

	if (len.z > len.y&& len.z > len.x)
		axis = 2;

	std::vector<Triangle> tempLeftTriangleList;
	std::vector<Triangle> tempRightTriangleList;

	auto splitByAxis = [&tempLeftTriangleList, &tempRightTriangleList, &midPoint, &vecTriangle](std::function<float(vec3 const& point)> getElement)
	{
		for (Triangle const& tri : vecTriangle)
		{
			if (getElement(tri.getCenter()) < getElement(midPoint))
				tempLeftTriangleList.push_back(tri);
			else
				tempRightTriangleList.push_back(tri);
		}
		assert(tempLeftTriangleList.size());
		assert(tempRightTriangleList.size());
	};

	using namespace std::placeholders;

	if (axis == 0)
		splitByAxis(bind(&vec3::x, _1));

	if (axis == 1)
		splitByAxis(bind(&vec3::y, _1));

	if (axis == 2)
		splitByAxis(bind(&vec3::z, _1));

	if (tempLeftTriangleList.size() == 1)
	{
		node.leftChild = tempLeftTriangleList[0].getIndex();
		node.childIsTriangle = 1;
		//node.leftChildIsTriangle = true;
	}
	else
	{
		node.leftChild = nodeList.size();
		nodeList.emplace_back();
		buildRecurcive(nodeList.size() - 1, tempLeftTriangleList);
	}

	if (tempRightTriangleList.size() == 1)
	{
		node.rightChild = tempRightTriangleList[0].getIndex();
		node.childIsTriangle = 2;
		//node.rightChildIsTriangle = true;
	}
	else
	{
		node.rightChild = nodeList.size();
		nodeList.emplace_back();
		buildRecurcive(nodeList.size() - 1, tempRightTriangleList);
	}
}

bool BVHBuilder::travelRecurcive(Node& node, glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT)
{
	if (!node.aabb.rayIntersect(origin, direction, minT))
		return false;

	if ((int)ceil(node.childIsTriangle) & 2)
		if (vecTriangle.at(abs(node.rightChild)).rayIntersect(origin, direction, color, minT))
			return true;

	if ((int)ceil(node.childIsTriangle) & 1)
		if (vecTriangle.at(abs(node.leftChild)).rayIntersect(origin, direction, color, minT))
			return true;

	if (((int)ceil(node.childIsTriangle) & 2) == 0)
		if (travelRecurcive(nodeList[node.rightChild], origin, direction, color, minT))
			return true;

	if (((int)ceil(node.childIsTriangle) & 1) == 0)
		if (travelRecurcive(nodeList[node.leftChild], origin, direction, color, minT))
			return true;

	return false;
}

bool BVHBuilder::travelStack(Node& node, glm::vec3& origin, glm::vec3& direction, glm::vec3& color, float& minT)
{
	std::stack<int> stack;
	stack.push(0);
	Node select;
	Triangle tri;
	float tempt;

	while (stack.size() != 0)
	{
		select = nodeList.at(stack.top());
		stack.pop();

		if (!select.aabb.rayIntersect(origin, direction, minT))
			continue;

		if (select.childIsTriangle == 0)
		{
			float leftMinT = 0;
			float rightMinT = 0;
			Node right = nodeList.at(select.rightChild);
			Node left = nodeList.at(select.leftChild);
			bool rightI = right.aabb.rayIntersect(origin, direction, rightMinT);
			bool leftI = left.aabb.rayIntersect(origin, direction, rightMinT);

			if (rightI)
				stack.push(select.rightChild);
			if (leftI)
				stack.push(select.leftChild);
			continue;
		}

		if (((int)ceil(select.childIsTriangle) & 2) == 0)
			stack.push(select.rightChild);

		if (((int)ceil(select.childIsTriangle) & 1) == 0)
			stack.push(select.leftChild);

		if (((int)ceil(select.childIsTriangle) & 2) > 0)
		{
			tri = vecTriangle.at(select.rightChild);
			tri.rayIntersect(origin, direction, color, minT);
		}

		if (((int)ceil(select.childIsTriangle) & 1) > 0)
		{
			tri = vecTriangle.at(select.leftChild);
			tri.rayIntersect(origin, direction, color, minT);
		}
	}
	return false;
}
