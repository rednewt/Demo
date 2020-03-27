#include "pch.h"
#include "DemoScene.h"
#include "Shaders/simpleVS.h"
#include "Shaders/simplePS.h"
#include "DDSTextureLoader.h"
#include "GeometricPrimitive.h"


using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_CBPerObjectData = {};
	m_DrawableCube = std::make_unique<Drawable>();
	m_DrawableCylinder = std::make_unique<Drawable>();

	m_CBPerFrameData.Light.Direction = XMFLOAT3(1.0f, -1.0f, 0.0f);
}

bool DemoScene::CreateDeviceDependentResources()
{
	if FAILED(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, GeometricPrimitive::VertexType::InputElementCount, g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreateVertexShader(g_byteArraySimpleVS, sizeof(g_byteArraySimpleVS), nullptr, m_SimpleVertexShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreatePixelShader(g_byteArraySimplePS, sizeof(g_byteArraySimplePS), nullptr, m_SimplePixelShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"crate.dds", 0, m_SRVCube.ReleaseAndGetAddressOf()))
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
	
	GeometricPrimitive::CreateCylinder(vertices, indices, 3.0f, 1.0f, 32U, false);
	m_DrawableCylinder->Create(m_Device.Get(), vertices, indices);
	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(3.0f, 3.0f, 3.0f), false);
	m_DrawableCube->Create(m_Device.Get(), vertices, indices);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(VS_ConstantBufferPerObject);

	D3D11_SUBRESOURCE_DATA cbData = {};
	cbData.pSysMem = &m_CBPerObjectData;

	if (FAILED(m_Device->CreateBuffer(&cbDesc, &cbData, m_CBPerObject.ReleaseAndGetAddressOf())))
		return false;

	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(PS_ConstantBufferPerFrame);

	cbData = {};
	cbData.pSysMem = &m_CBPerFrameData;

	if (FAILED(m_Device->CreateBuffer(&cbDesc, &cbData, m_CBPerFrame.ReleaseAndGetAddressOf())))
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

	XMMATRIX worldCube= XMMatrixRotationX(XMConvertToRadians(angle));
	worldCube *= XMMatrixRotationAxis(XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f), XMConvertToRadians(angle));
	worldCube *= XMMatrixTranslation(-2, 0, 0);

	XMMATRIX worldCylinder = XMMatrixRotationY(XMConvertToRadians(angle))  * XMMatrixTranslation(2, 0, 0);
	
	XMMATRIX viewProj = view * proj;

	XMMATRIX WVPCube = worldCube * viewProj;
	XMMATRIX WVPCylinder = worldCylinder * viewProj;

	XMStoreFloat4x4(&m_DrawableCube->WorldTransform, worldCube);
	XMStoreFloat4x4(&m_DrawableCube->WorldViewProjTransform, WVPCube);
	XMStoreFloat4x4(&m_DrawableCylinder->WorldTransform, worldCylinder);
	XMStoreFloat4x4(&m_DrawableCylinder->WorldViewProjTransform, WVPCylinder);
	
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
	m_ImmediateContext->PSSetShaderResources(0, 1, m_SRVCube.GetAddressOf());
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_CBPerObject.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(0, 1, m_CBPerFrame.GetAddressOf());

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;
	
	//Set Cube transform
	D3D11_MAPPED_SUBRESOURCE mappedRes = {};
	m_ImmediateContext->Map(m_CBPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
	m_CBPerObjectData.WorldViewProj = m_DrawableCube->WorldViewProjTransform;
	m_CBPerObjectData.World = m_DrawableCube->WorldTransform;
	m_CBPerObjectData.WorldInverseTranspose = Helpers::ComputeInverseTranspose(m_DrawableCube->WorldTransform);
	memcpy(mappedRes.pData, &m_CBPerObjectData, sizeof(m_CBPerObjectData));
	m_ImmediateContext->Unmap(m_CBPerObject.Get(), 0);

	//Draw Cube
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableCube->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_DrawableCube->IndexBuffer.Get(), m_DrawableCube->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_DrawableCube->IndexCount, 0, 0);

	//Set Cylinder Transform
	m_ImmediateContext->Map(m_CBPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
	m_CBPerObjectData.WorldViewProj = m_DrawableCylinder->WorldViewProjTransform;
	m_CBPerObjectData.World = m_DrawableCylinder->WorldTransform;
	m_CBPerObjectData.WorldInverseTranspose = Helpers::ComputeInverseTranspose(m_DrawableCylinder->WorldTransform);
	memcpy(mappedRes.pData, &m_CBPerObjectData, sizeof(m_CBPerObjectData));
	m_ImmediateContext->Unmap(m_CBPerObject.Get(), 0); 

	//Draw Cylinder
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableCylinder->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_DrawableCylinder->IndexBuffer.Get(), m_DrawableCylinder->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_DrawableCylinder->IndexCount, 0, 0);

	
	m_SwapChain->Present(0, 0);
}

