#include "pch.h"
#include "DemoScene.h"
#include "Shaders/simpleVS.h"
#include "Shaders/simplePS.h"

using namespace DirectX;
using namespace VertexStructure;

DemoScene::DemoScene(const HWND& hwnd) :
	DemoBase(hwnd)
{
	m_SimpleVertexDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 };
	m_SimpleVertexDesc[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 };
}

bool DemoScene::Initialize()
{
	if (!DemoBase::Initialize())
		return false;
	
	if FAILED(m_Device->CreateInputLayout(m_SimpleVertexDesc, ARRAYSIZE(m_SimpleVertexDesc), g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreateVertexShader(g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), nullptr, m_SimpleVertexShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreatePixelShader(g_byteArraySimplePS, sizeof(g_byteArraySimplePS), nullptr, m_SimplePixelShader.ReleaseAndGetAddressOf()))
		return false;

	SimpleVertex vertices[3] =
	{
		{ XMFLOAT3(0.0f, 0.8f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }
	};

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vbDesc.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vbData = {};
	vbData.pSysMem = vertices;

	if FAILED(m_Device->CreateBuffer(&vbDesc, &vbData, m_SimpleVertexBuffer.ReleaseAndGetAddressOf()))
		return false;

	return true;
}

DemoScene::~DemoScene()
{
}

void DemoScene::UpdateScene(float dt)
{
}

void DemoScene::DrawScene()
{
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), DirectX::Colors::Black);
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	m_ImmediateContext->IASetInputLayout(m_SimpleVertexLayout.Get());
	m_ImmediateContext->VSSetShader(m_SimpleVertexShader.Get(), nullptr, 0);
	m_ImmediateContext->PSSetShader(m_SimplePixelShader.Get(), nullptr, 0);
	
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_SimpleVertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	m_ImmediateContext->Draw(3, 0);

	m_SwapChain->Present(0, 0);
}
