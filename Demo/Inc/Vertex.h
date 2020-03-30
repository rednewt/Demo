#pragma once

struct SimpleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 Tex;

	static const int ElementCount = 2;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};
