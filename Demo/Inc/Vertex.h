#pragma once

struct SimpleVertex
{
	DirectX::XMFLOAT3 Position;

	static const int ElementCount = 1;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};

struct TreePointSprite
{
	TreePointSprite(const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT2 size) :
		Center(center), Size(size)
	{}

	DirectX::XMFLOAT3 Center;
	DirectX::XMFLOAT2 Size;

	static const int ElementCount = 2;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};