#pragma once
#include <vector>
#include <string>
namespace ModelLoader
{
	void Obj(std::string const& filePath, std::vector<float>& rawVertex, std::vector<float>& rawNormal, std::vector<float>& rawUV);
};

