#include "pch.h"
#include "Drawable.h"
#include "Vertex.h"
#include <GeometricPrimitive.h>

using namespace DirectX;
using Microsoft::WRL::ComPtr;

ComPtr<ID3D11VertexShader> SimpleDrawable::m_VertexShader = nullptr;
ComPtr<ID3D11PixelShader> SimpleDrawable::m_PixelShader = nullptr;
ComPtr<ID3D11InputLayout> SimpleDrawable::m_InputLayout = nullptr;
ComPtr<ID3D11Buffer> SimpleDrawable::m_CbConstants = nullptr;

namespace
{
	#include "Shaders\Compiled\DebugPS.h"
	#include "Shaders\Compiled\DebugVS.h"
}

SimpleDrawable::SimpleDrawable() :
	m_IndexCount(0)
{
	m_CbConstantsData = {};
}

//TODO:
//Save Memory --
//Since only limited number of primitives (Shapes) can be drawn, there is no point in
//creating different vertex/index/constant buffers each time in Create()

void SimpleDrawable::CreateDeviceDependentResources(ID3D11Device* const device)
{
	if (!m_VertexShader.Get())
		device->CreateVertexShader(g_SimpleVS, sizeof(g_SimpleVS), nullptr, m_VertexShader.ReleaseAndGetAddressOf());

	if (!m_PixelShader.Get())
		device->CreatePixelShader(g_SimplePS, sizeof(g_SimplePS), nullptr, m_PixelShader.ReleaseAndGetAddressOf());

	if (!m_CbConstants.Get())
		Helpers::CreateConstantBuffer<VS_PS_CbConstants>(device, m_CbConstants.ReleaseAndGetAddressOf());
}

std::unique_ptr<SimpleDrawable> SimpleDrawable::Create(ID3D11Device* device)
{
	assert(device != nullptr);
	std::unique_ptr<SimpleDrawable> pObj(new SimpleDrawable());

	pObj->CreateDeviceDependentResources(device);

	return pObj;
}

std::unique_ptr<SimpleDrawable> SimpleDrawable::Create(ID3D11Device* device, Shape shape)
{
	assert(device != nullptr);
	std::unique_ptr<SimpleDrawable> pObj(new SimpleDrawable());

	pObj->CreateDeviceDependentResources(device);

	std::vector<uint16_t> indices;
	std::vector<VertexPositionNormalTexture> vertices;

	switch (shape)
	{
	case Shape::Cube:	
		GeometricPrimitive::CreateCube(vertices, indices, 0.2f, false);
		break;
	case Shape::Sphere:
		GeometricPrimitive::CreateSphere(vertices, indices, 0.3f, 16, false);
		break;
	default:
		return nullptr;
	}

	pObj->m_IndexCount = static_cast<uint16_t>(indices.size());
	Helpers::CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, pObj->m_VertexBuffer.ReleaseAndGetAddressOf());
	Helpers::CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, pObj->m_IndexBuffer.ReleaseAndGetAddressOf());

	return pObj;
}

void SimpleDrawable::BindShader(ID3D11DeviceContext* context, ID3D11InputLayout* posNormalTexture)
{
	assert(context != nullptr);

	context->IASetInputLayout(posNormalTexture);
	context->VSSetShader(SimpleDrawable::m_VertexShader.Get(), nullptr, 0);
	context->PSSetShader(SimpleDrawable::m_PixelShader.Get(), nullptr, 0);

	context->VSSetConstantBuffers(0, 1, SimpleDrawable::m_CbConstants.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, SimpleDrawable::m_CbConstants.GetAddressOf());

	if (m_VertexBuffer == nullptr || m_IndexBuffer == nullptr)
		return;

	UINT stride = sizeof(VertexPositionNormalTexture);
	UINT offset = 0;
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
}
void SimpleDrawable::UpdateConstantBuffer(ID3D11DeviceContext* context, DirectX::FXMMATRIX worldViewProj, DirectX::FXMVECTOR objectColor)
{
	assert(context != nullptr);

	XMStoreFloat4x4(&m_CbConstantsData.WorldViewProj, worldViewProj);
	XMStoreFloat4(&m_CbConstantsData.Color, objectColor);

	Helpers::UpdateConstantBuffer(context, m_CbConstants.Get(), &m_CbConstantsData);
}
void SimpleDrawable::Draw(ID3D11DeviceContext* context)
{
	assert(context != nullptr);
	assert(m_IndexCount != 0);
	
	context->DrawIndexed(m_IndexCount, 0, 0);
}
