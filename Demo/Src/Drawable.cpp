#include "pch.h"
#include "Drawable.h"
#include "Vertex.h"
#include <GeometricPrimitive.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

ComPtr<ID3D11VertexShader> DebugDrawable::m_VertexShader = nullptr;
ComPtr<ID3D11PixelShader> DebugDrawable::m_PixelShader = nullptr;
ComPtr<ID3D11InputLayout> DebugDrawable::m_InputLayout = nullptr;

namespace
{
	#include "Shaders\Compiled\DebugPS.h"
	#include "Shaders\Compiled\DebugVS.h"
}

std::unique_ptr<DebugDrawable> DebugDrawable::Create(ID3D11Device* device, Shape shape)
{
	assert(device != nullptr);

	if (!m_InputLayout.Get())
		device->CreateInputLayout(SimpleVertex::InputElements, SimpleVertex::ElementCount, g_DebugVS, sizeof(g_DebugVS), m_InputLayout.ReleaseAndGetAddressOf());
	if (!m_VertexShader.Get())
		device->CreateVertexShader(g_DebugVS, sizeof(g_DebugVS), nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	if (!m_PixelShader.Get())
		device->CreatePixelShader(g_DebugPS, sizeof(g_DebugPS), nullptr, m_PixelShader.ReleaseAndGetAddressOf());

	std::unique_ptr<DebugDrawable> pObj(new DebugDrawable());
	
	Helpers::CreateConstantBuffer<VS_PS_CbConstants>(device, pObj->m_CbConstants.ReleaseAndGetAddressOf());

	std::vector<SimpleVertex> vertices;
	std::vector<uint16_t> indices;
	std::vector<VertexPositionNormalTexture> tempVertices;

	switch (shape)
	{
	case Shape::CUBE:	
		GeometricPrimitive::CreateCube(tempVertices, indices, 0.5f, false);
		break;
	case Shape::SPHERE:
		GeometricPrimitive::CreateSphere(tempVertices, indices, 0.5f, 16, false);
		break;
	default:
		return nullptr;
	}

	pObj->IndexCount = static_cast<uint16_t>(indices.size());

	for (auto const& vertex : tempVertices)
	{
		SimpleVertex v;
		v.Position = vertex.position;
		vertices.push_back(v);
	}

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SimpleVertex) * vertices.size();
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices.data();

	DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData, pObj->m_VertexBuffer.ReleaseAndGetAddressOf()));

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(uint16_t) * indices.size();
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices.data();

	DX::ThrowIfFailed(device->CreateBuffer(&ibDesc, &ibData, pObj->m_IndexBuffer.ReleaseAndGetAddressOf()));

	return pObj;
}

void DebugDrawable::Draw(ID3D11DeviceContext* context, FXMMATRIX worldViewProj, FXMVECTOR objectColor)
{
	assert(context != nullptr);

	XMStoreFloat4x4(&m_CbConstantsData.WorldViewProj, worldViewProj);
	XMStoreFloat4(&m_CbConstantsData.Color, objectColor);

	Helpers::UpdateConstantBuffer<VS_PS_CbConstants>(context, m_CbConstants.Get(), &m_CbConstantsData);

	context->IASetInputLayout(DebugDrawable::m_InputLayout.Get());
	context->VSSetShader(DebugDrawable::m_VertexShader.Get(), nullptr, 0);
	context->PSSetShader(DebugDrawable::m_PixelShader.Get(), nullptr, 0);

	context->VSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->DrawIndexed(IndexCount, 0, 0);
}
