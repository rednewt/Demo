#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
//These headers may not be present, so you may need to build the project first
#include "Shaders\Compiled\BasicShaderPS.h"
#include "Shaders\Compiled\BasicShaderVS.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_CBPerObjectData = {};
	m_CBPerFrameData = {};

	//Setup DirectionalLight
	m_CBPerFrameData.DirLight.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//Setup Pointlight
	m_CBPerFrameData.PointLight.Position = XMFLOAT3(1.0f, 2.0f, 0.0f);
	//Setup Spotlight
	m_CBPerFrameData.SpotLight.Position = XMFLOAT3(0.0f, 0.0f, -3.0f);
	m_CBPerFrameData.SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_DrawableBox = std::make_unique<Drawable>();
	m_DrawableSphere = std::make_unique<Drawable>();

	m_DrawableBox->Material.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_DrawableSphere->Material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DrawableSphere->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 64.0f);
}

bool DemoScene::CreateDeviceDependentResources()
{
	if FAILED(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, 
		GeometricPrimitive::VertexType::InputElementCount, 
		g_VSBasicShader, sizeof(g_VSBasicShader), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreateVertexShader(g_VSBasicShader, sizeof(g_VSBasicShader), nullptr, m_SimpleVertexShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreatePixelShader(g_PSBasicShader, sizeof(g_PSBasicShader), nullptr, m_SimplePixelShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\crate.dds", 0, m_SRVCube.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_SRVCylinder.ReleaseAndGetAddressOf()))
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
	
	GeometricPrimitive::CreateSphere(vertices, indices, 3.0f, 32, false);
	m_DrawableSphere->Create(m_Device.Get(), vertices, indices);
	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(3.0f, 3.0f, 3.0f), false);
	m_DrawableBox->Create(m_Device.Get(), vertices, indices);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(VS_PS_ConstantBufferPerObject);

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
	ImGui_NewFrame();

	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 0.1f, 200.0f);
	XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR focus = XMVectorSet(0, 0, 0, 1);
	XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);
	
	static float angle = 0.0f;
	angle += 45.0f * dt;

	XMMATRIX RotateY = XMMatrixRotationY(XMConvertToRadians(angle));

	XMMATRIX worldCube = RotateY * XMMatrixTranslation(-2, 0, 0);
	XMMATRIX worldCylinder = RotateY * XMMatrixTranslation(2, 0, 0);
	
	XMMATRIX viewProj = view * proj;
	XMMATRIX WVPCube = worldCube * viewProj;
	XMMATRIX WVPCylinder = worldCylinder * viewProj;

	XMStoreFloat4x4(&m_DrawableBox->WorldTransform, worldCube);
	XMStoreFloat4x4(&m_DrawableBox->WorldViewProjTransform, WVPCube);
	XMStoreFloat4x4(&m_DrawableSphere->WorldTransform, worldCylinder);
	XMStoreFloat4x4(&m_DrawableSphere->WorldViewProjTransform, WVPCylinder);
	

	static XMFLOAT3 DirLightVector = XMFLOAT3(1.0f, 0.0f, 0.0f);

	D3D11_MAPPED_SUBRESOURCE mappedRes;
	m_ImmediateContext->Map(m_CBPerFrame.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);

	XMStoreFloat3(&m_CBPerFrameData.EyePos, eyePos);
	m_CBPerFrameData.DirLight.SetDirection(DirLightVector);

	memcpy(mappedRes.pData, &m_CBPerFrameData, sizeof(m_CBPerFrameData));

	m_ImmediateContext->Unmap(m_CBPerFrame.Get(), 0);

#pragma region ImGui Widgets
	if (ImGui::Begin("Scene"))
	{
		if (ImGui::CollapsingHeader("Lights"))
		{
			if (ImGui::TreeNode("Directional Light"))
			{
				ImGui::ColorEdit4("Ambient##1", reinterpret_cast<float*>(&m_CBPerFrameData.DirLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##1", reinterpret_cast<float*>(&m_CBPerFrameData.DirLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##1", reinterpret_cast<float*>(&m_CBPerFrameData.DirLight.Specular), 2);
				ImGui::InputFloat3("Direction##1", reinterpret_cast<float*>(&DirLightVector), 2);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Point Light"))
			{
				ImGui::ColorEdit4("Ambient##2", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##2", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##2", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Specular), 2);
				ImGui::InputFloat3("Position##1", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Position), 2);
				ImGui::InputFloat("Range##1", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Range), 2);
				ImGui::InputFloat3("Attenuation##1", reinterpret_cast<float*>(&m_CBPerFrameData.PointLight.Attenuation), 2);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Spot Light"))
			{
				ImGui::ColorEdit4("Ambient##5", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Ambient), 2);
				ImGui::ColorEdit4("Diffuse##5", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Diffuse), 2);
				ImGui::ColorEdit3("Specular##5", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Specular), 2);
				ImGui::InputFloat3("Position##2", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Position), 2);
				ImGui::InputFloat("Range##2", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Range), 2);
				ImGui::InputFloat("SpotPower", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.SpotPower), 2);
				ImGui::InputFloat3("Direction##2", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Direction), 2);
				ImGui::InputFloat3("Attenuation##2", reinterpret_cast<float*>(&m_CBPerFrameData.SpotLight.Attenuation), 2);

				ImGui::TreePop();
			}
		}

		if (ImGui::CollapsingHeader("Scene Objects"))
		{
			if (ImGui::TreeNode("Sphere"))
			{
				ImGui::InputFloat4("Ambient##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Ambient), 2);
				ImGui::InputFloat4("Diffuse##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Diffuse), 2);
				ImGui::InputFloat4("Specular##3", reinterpret_cast<float*>(&m_DrawableSphere->Material.Specular), 2);

				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Box"))
			{
				ImGui::InputFloat4("Ambient##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Ambient), 2);
				ImGui::InputFloat4("Diffuse##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Diffuse), 2);
				ImGui::InputFloat4("Specular##4", reinterpret_cast<float*>(&m_DrawableBox->Material.Specular), 2);

				ImGui::TreePop();
			}
		}
	
		ImGui::End();
	}
#pragma endregion
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

	//Set CBPerObject for VS & PS
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_CBPerObject.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(0, 1, m_CBPerObject.GetAddressOf());
	//Set CBPerFrame for PS
	m_ImmediateContext->PSSetConstantBuffers(1, 1, m_CBPerFrame.GetAddressOf());

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;
	
	//Set Cube transform & Material
	m_CBPerObjectData.WorldViewProj = m_DrawableBox->WorldViewProjTransform;
	m_CBPerObjectData.World = m_DrawableBox->WorldTransform;
	m_CBPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(m_DrawableBox->WorldTransform);
	m_CBPerObjectData.Material = m_DrawableBox->Material;
	D3D11_MAPPED_SUBRESOURCE mappedRes = {};
	m_ImmediateContext->Map(m_CBPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
	memcpy(mappedRes.pData, &m_CBPerObjectData, sizeof(m_CBPerObjectData));
	m_ImmediateContext->Unmap(m_CBPerObject.Get(), 0);
	
	//Draw Cube
	m_ImmediateContext->PSSetShaderResources(0, 1, m_SRVCube.GetAddressOf());
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableBox->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_DrawableBox->IndexBuffer.Get(), m_DrawableBox->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_DrawableBox->IndexCount, 0, 0);

	//Set Cylinder Transform & Material
	m_CBPerObjectData.WorldViewProj = m_DrawableSphere->WorldViewProjTransform;
	m_CBPerObjectData.World = m_DrawableSphere->WorldTransform;
	m_CBPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(m_DrawableSphere->WorldTransform);
	m_CBPerObjectData.Material = m_DrawableSphere->Material;
	m_ImmediateContext->Map(m_CBPerObject.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes);
	memcpy(mappedRes.pData, &m_CBPerObjectData, sizeof(m_CBPerObjectData));
	m_ImmediateContext->Unmap(m_CBPerObject.Get(), 0); 

	//Draw Cylinder
	m_ImmediateContext->PSSetShaderResources(0, 1, m_SRVCylinder.GetAddressOf());
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableSphere->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_DrawableSphere->IndexBuffer.Get(), m_DrawableSphere->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_DrawableSphere->IndexCount, 0, 0);


	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_SwapChain->Present(1, 0);
}

