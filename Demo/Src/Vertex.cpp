#include "pch.h"
#include "Vertex.h"

const D3D11_INPUT_ELEMENT_DESC SimpleVertex::InputElements[SimpleVertex::ElementCount] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static_assert(sizeof(SimpleVertex) == 12, "mismatch");

const D3D11_INPUT_ELEMENT_DESC TreePointSprite::InputElements[TreePointSprite::ElementCount] =
{
	{ "CENTER", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SIZE",   0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static_assert(sizeof(TreePointSprite) == 20, "mismatch");
