#include "pch.h"
#include "DemoScene.h"
#include "Vertex.h"
#include "Shaders/simpleVS.h"
#include "Shaders/simplePS.h"
#include "DDSTextureLoader.h"



using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_SimpleCBData = {};
	m_Cube = std::make_unique<Drawable>();
}

bool DemoScene::CreateDeviceDependentResources()
{
	if FAILED(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, GeometricPrimitive::VertexType::InputElementCount, g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreateVertexShader(g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), nullptr, m_SimpleVertexShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreatePixelShader(g_byteArraySimplePS, sizeof(g_byteArraySimplePS), nullptr, m_SimplePixelShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"crate.dds", 0, m_CubeTexSRV.ReleaseAndGetAddressOf()))
		return false;

	if (!CreateBuffers())
		return false;

	CD3D11_DEFAULT def;
	CD3D11_SAMPLER_DESC desc(def);

	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.MaxAnisotropy = 8;

	if FAILED(m_Device->CreateSamplerState(&desc, m_SamplerDefault.ReleaseAndGetAddressOf()))
		return false;

	return true;
}

bool DemoScene::CreateBuffers()
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;
	GeometricPrimitive::CreateCube(vertices, indices, 3.0f, false);
	m_Cube->Create(m_Device.Get(), vertices, indices);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(SimpleConstantBuffer);

	D3D11_SUBRESOURCE_DATA cbData = {};
	cbData.pSysMem = &m_SimpleCBData;

	if (FAILED(m_Device->CreateBuffer(&cbDesc, &cbData, m_SimpleConstantBuffer.ReleaseAndGetAddressOf())))
		return false;

	return true;
}

bool DemoScene::Initialize()
{
	if (!Super::Initialize())
		return false;

	if (!CreateDeviceDependentResources())
		return false;

	m_ImmediateContext->IASetInputLayout(m_SimpleVertexLayout.Get());
	m_ImmediateContext->VSSetShader(m_SimpleVertexShader.Get(), nullptr, 0);
	m_ImmediateContext->PSSetShader(m_SimplePixelShader.Get(), nullptr, 0);
	m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerDefault.GetAddressOf());

	return true;
}

void DemoScene::UpdateScene(float dt)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 0.1f, 200.0f);
	
	XMVECTOR eyePos = XMVectorSet(0, 0, -10.0f, 1.0f);
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR focus = XMVectorSet(0, 0, 0, 1);
	XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);
	
	static float angle = 0.0f;
	angle += 10.0f * dt;

	XMMATRIX world = XMMatrixRotationX(XMConvertToRadians(angle));
	world *= XMMatrixRotationAxis(XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f), XMConvertToRadians(angle));
	
	XMMATRIX worldViewProj = world * view * proj;

	D3D11_MAPPED_SUBRESOURCE mappedRes = {};
	m_ImmediateContext->Map(m_SimpleConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);

	XMStoreFloat4x4(&m_SimpleCBData.WorldViewProj, worldViewProj);
	memcpy(mappedRes.pData, &m_SimpleCBData, sizeof(m_SimpleCBData));

	m_ImmediateContext->Unmap(m_SimpleConstantBuffer.Get(), 0);
}

void DemoScene::Clear()
{
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), DirectX::Colors::Black);
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DemoScene::DrawScene()
{
	Clear();

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_SimpleConstantBuffer.GetAddressOf());
	m_ImmediateContext->PSSetShaderResources(0, 1, m_CubeTexSRV.GetAddressOf());

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_Cube->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_Cube->IndexBuffer.Get(), m_Cube->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_Cube->IndexCount, 0, 0);
	
	m_SwapChain->Present(0, 0);
}

