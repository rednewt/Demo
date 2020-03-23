#include "pch.h"
#include "DemoScene.h"
#include "Shaders/simpleVS.h"
#include "Shaders/simplePS.h"
#include "DDSTextureLoader.h"

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	DemoBase(hwnd)
{
	m_SimpleVertexDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	m_SimpleVertexDesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };

	m_SimpleCBData = {};
}

bool DemoScene::CreateDeviceDependentResources()
{
	if FAILED(m_Device->CreateInputLayout(m_SimpleVertexDesc, ARRAYSIZE(m_SimpleVertexDesc), g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
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

	if FAILED(m_Device->CreateSamplerState(&desc, m_SamplerDefault.ReleaseAndGetAddressOf()))
		return false;

	return true;
}

bool DemoScene::CreateBuffers()
{
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }, //
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //

		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //

		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //

		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) }, //
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, //

		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) }, //
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }, //
	};

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;

	if FAILED(m_Device->CreateBuffer(&vbDesc, &vbData, m_CubeVB.ReleaseAndGetAddressOf()))
		return false;

	UINT indices[] =
	{
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	};

	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
	ibDesc.ByteWidth = sizeof(UINT) * ARRAYSIZE(indices);
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA ibData = {};
	ibData.pSysMem = indices;

	if FAILED(m_Device->CreateBuffer(&ibDesc, &ibData, m_CubeIB.ReleaseAndGetAddressOf()))
		return false;


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
	if (!DemoBase::Initialize())
		return false;

	if (!CreateDeviceDependentResources())
		return false;

	m_ImmediateContext->IASetInputLayout(m_SimpleVertexLayout.Get());
	m_ImmediateContext->VSSetShader(m_SimpleVertexShader.Get(), nullptr, 0);
	m_ImmediateContext->PSSetShader(m_SimplePixelShader.Get(), nullptr, 0);

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
	angle += 45.0f * dt;
	XMMATRIX world = XMMatrixScaling(2.0f, 2.0f, 2.0f) * XMMatrixRotationY(XMConvertToRadians(angle));
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
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DemoScene::DrawScene()
{
	Clear();

	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_SimpleConstantBuffer.GetAddressOf());
	m_ImmediateContext->PSSetShaderResources(0, 1, m_CubeTexSRV.GetAddressOf());
	m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerDefault.GetAddressOf());

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_CubeVB.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_CubeIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_ImmediateContext->DrawIndexed(36, 0, 0);

	m_SwapChain->Present(0, 0);
}

DemoScene::~DemoScene()
{
}