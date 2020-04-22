#include "pch.h"

using namespace DirectX;

void Helpers::CreateGrid(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices, 
	const uint16_t width, const uint16_t depth, uint16_t tessellation)
{
	vertices.clear();
	indices.clear();

	UINT vertexCount = tessellation * tessellation;
	UINT faceCount = (tessellation - 1) * (tessellation - 1) * 2;

	float quadSizeX = width / (tessellation - 1);
	float quadSizeZ = depth / (tessellation - 1);

	float quadU = 1.0f / (tessellation - 1);
	float quadV = 1.0f / (tessellation - 1);

	float halfWidth = 0.5f * static_cast<float>(width);
	float halfDepth = 0.5f * static_cast<float>(depth);

	vertices.resize(vertexCount);

	for (int i = 0; i < tessellation; ++i)
	{
		float z = halfDepth - i * quadSizeZ;

		for (int j = 0; j < tessellation; ++j)
		{
			float x = -halfWidth + j * quadSizeX;

			vertices[i * tessellation + j].position = XMFLOAT3(x, 0.0f, z);
			vertices[i * tessellation + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertices[i * tessellation + j].textureCoordinate = XMFLOAT2(quadU * j, quadV * i);
		}
	}

	indices.resize(faceCount * 3);

	UINT k = 0;
	for (int i = 0; i < tessellation; ++i)
	{
		for (int j = 0; j < tessellation; ++j)
		{
		       indices[k] = i * tessellation + j;
		       indices[k + 1] = i * tessellation + j + 1;
		       indices[k + 2] = (i + 1) * tessellation + j;
		       indices[k + 3] = (i + 1) * tessellation + j;
		       indices[k + 4] = i * tessellation + j + 1;
		       indices[k + 5] = (i + 1) * tessellation + j + 1;
				
			k += 6;
		}
	}
}
