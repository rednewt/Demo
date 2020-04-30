#include "pch.h"
#include "Drawable.h"
#include "Vertex.h"
#include <GeometricPrimitive.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

ComPtr<ID3D11VertexShader> DebugDrawable::m_VertexShader = nullptr;
ComPtr<ID3D11PixelShader> DebugDrawable::m_PixelShader = nullptr;
ComPtr<ID3D11InputLayout> DebugDrawable::m_InputLayout = nullptr;
ComPtr<ID3D11Buffer> DebugDrawable::m_CbConstants = nullptr;

namespace
{
	#include "Shaders\Compiled\DebugPS.h"
	#include "Shaders\Compiled\DebugVS.h"
}

DebugDrawable::DebugDrawable() :
	m_IndexCount(0)
{
	m_CbConstantsData = {};
}

//TODO:
//Save Memory --
//Since only limited number of primitives (Shapes) can be drawn, there is no point in
//creating different vertex/index/constant buffers each time in Create()

std::unique_ptr<DebugDrawable> DebugDrawable::Create(ID3D11Device* device, Shape shape)
{
	assert(device != nullptr);

	if (!m_InputLayout.Get())
		device->CreateInputLayout(SimpleVertex::InputElements, SimpleVertex::ElementCount, g_DebugVS, sizeof(g_DebugVS), m_InputLayout.ReleaseAndGetAddressOf());
	
	if (!m_VertexShader.Get())
		device->CreateVertexShader(g_DebugVS, sizeof(g_DebugVS), nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	
	if (!m_PixelShader.Get())
		device->CreatePixelShader(g_DebugPS, sizeof(g_DebugPS), nullptr, m_PixelShader.ReleaseAndGetAddressOf());

	if (!m_CbConstants.Get())
		Helpers::CreateConstantBuffer<VS_PS_CbConstants>(device, m_CbConstants.ReleaseAndGetAddressOf());

	std::vector<SimpleVertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<VertexPositionNormalTexture> tempVertices;

	switch (shape)
	{
	case Shape::Cube:	
		GeometricPrimitive::CreateCube(tempVertices, indices, 0.2f, false);
		break;
	case Shape::Sphere:
		GeometricPrimitive::CreateSphere(tempVertices, indices, 0.3f, 16, false);
		break;
	default:
		return nullptr;
	}

	for (auto const& vertex : tempVertices)
	{
		SimpleVertex v;
		v.Position = vertex.position;
		vertices.push_back(v);
	}

	std::unique_ptr<DebugDrawable> pObj(new DebugDrawable());
	pObj->m_IndexCount = static_cast<uint16_t>(indices.size());
	Helpers::CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, pObj->m_VertexBuffer.ReleaseAndGetAddressOf());
	Helpers::CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, pObj->m_IndexBuffer.ReleaseAndGetAddressOf());

	return pObj;
}

void DebugDrawable::PrepareForRendering(ID3D11DeviceContext* context)
{
	context->IASetInputLayout(DebugDrawable::m_InputLayout.Get());
	context->VSSetShader(DebugDrawable::m_VertexShader.Get(), nullptr, 0);
	context->PSSetShader(DebugDrawable::m_PixelShader.Get(), nullptr, 0);

	context->VSSetConstantBuffers(0, 1, DebugDrawable::m_CbConstants.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, DebugDrawable::m_CbConstants.GetAddressOf());

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
}

void DebugDrawable::Draw(ID3D11DeviceContext* context, FXMMATRIX worldViewProj, FXMVECTOR objectColor)
{
	assert(context != nullptr);

	XMStoreFloat4x4(&m_CbConstantsData.WorldViewProj, worldViewProj);
	XMStoreFloat4(&m_CbConstantsData.Color, objectColor);

	Helpers::UpdateConstantBuffer(context, m_CbConstants.Get(), &m_CbConstantsData);

	context->DrawIndexed(m_IndexCount, 0, 0);
}
