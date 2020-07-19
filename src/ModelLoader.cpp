#include "ModelLoader.h"
#include <iostream>
#include <fstream>
#include "Utils.h"

void ModelLoader::Obj(std::string const& filePath, std::vector<float>& rawVertex, std::vector<float>& rawNormal, std::vector<float>& rawUV)
{
    rawVertex.clear();
    rawNormal.clear();
    rawUV.clear();

    std::vector<float> tempVertex;
    std::vector<float> tempNormal;
    std::vector<float> tempuv;

    auto setData = [](int startIndex, int count, std::vector<float>& sourceBuffer, std::vector<float>& forBuffer)
    {
        int vertexIndex = startIndex - 1;
        float* pVertex = &sourceBuffer[vertexIndex * count];
        forBuffer.push_back(pVertex[0]);
        forBuffer.push_back(pVertex[1]);
        if (count == 3)
            forBuffer.push_back(pVertex[2]);
    };

    std::ifstream stream(Utils::resourceDir + filePath);
    if (!stream.is_open())
    {
        std::cerr << "error load file " + filePath << std::endl;
        return;
    }

    float x, y, z;
    int v1, t1, n1, v2, t2, n2, v3, t3, n3;
    std::string s;

    while (getline(stream, s))
    {
        if (sscanf(s.c_str(), "v %f %f %f", &x, &y, &z))
        {
            tempVertex.push_back(x);
            tempVertex.push_back(y);
            tempVertex.push_back(z);
        }

        if (sscanf(s.c_str(), "vt %f %f", &x, &y))
        {
            tempuv.push_back(x);
            tempuv.push_back(y);
        }

        if (sscanf(s.c_str(), "vn %f %f %f", &x, &y, &z))
        {
            tempNormal.push_back(x);
            tempNormal.push_back(y);
            tempNormal.push_back(z);
        }

        if (sscanf(s.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3))
        {
            setData(v1, 3, tempVertex, rawVertex);
            setData(v2, 3, tempVertex, rawVertex);
            setData(v3, 3, tempVertex, rawVertex);

            setData(t1, 2, tempuv, rawUV);
            setData(t2, 2, tempuv, rawUV);
            setData(t3, 2, tempuv, rawUV);

            setData(n1, 3, tempNormal, rawNormal);
            setData(n2, 3, tempNormal, rawNormal);
            setData(n3, 3, tempNormal, rawNormal);
        }
    }
}
