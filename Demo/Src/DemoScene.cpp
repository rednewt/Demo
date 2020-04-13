#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

//These headers may not be present, so you may need to build the project once
namespace
{
	#include "Shaders\Compiled\BasicShaderPS.h"
	#include "Shaders\Compiled\BasicShaderVS.h"
}

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_CbPerObjectData = {};
	m_CbPerFrameData = {};

	m_DrawableBox = std::make_unique<Drawable>();
	m_DrawableSphere = std::make_unique<Drawable>();
	m_DrawableTorus = std::make_unique<Drawable>();
	m_DrawableTeapot = std::make_unique<Drawable>();

	//Setup DirectionalLight
	m_CbPerFrameData.DirLight.Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//Setup Pointlight
	m_CbPerFrameData.PointLight.Position = XMFLOAT3(0.0f, 2.0f, -2.0f);
	//Setup Spotlight
	m_CbPerFrameData.SpotLight.Position = XMFLOAT3(2.0f, -2.0f, -7.0f);
	m_CbPerFrameData.SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_CbPerFrameData.SpotLight.SpotPower = 2.0f;

	//Setup Materials
	m_DrawableBox->Material.Ambient = XMFLOAT4(0.35f, 0.35f, 0.35f, 1.0f);
	m_DrawableSphere->Material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DrawableSphere->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 64.0f);
	m_DrawableTorus->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 64.0f);
	m_DrawableTorus->Material.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
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

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\crate.dds", 0, m_DrawableBox->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableSphere->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\rock.jpg", 0, m_DrawableTorus->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\flooring.png", 0, m_DrawableTeapot->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if (!CreateBuffers())
		return false;
	
	CD3D11_DEFAULT def;
	CD3D11_SAMPLER_DESC desc(def);

	desc.Filter = D3D11_FILTER_ANISOTROPIC;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.MaxAnisotropy = 8;
	
	if FAILED(m_Device->CreateSamplerState(&desc, m_SamplerAnisotropic.ReleaseAndGetAddressOf()))
		return false;
	
	return true;
}

bool DemoScene::CreateBuffers()
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;

	GeometricPrimitive::CreateSphere(vertices, indices, 3.0f, 32, false);
	m_DrawableSphere->Create(m_Device.Get(), vertices, indices);
	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(2.5f, 2.5f, 2.5f), false);
	m_DrawableBox->Create(m_Device.Get(), vertices, indices);
	GeometricPrimitive::CreateTorus(vertices, indices, 3.0f, 1.0f, 64, false);
	m_DrawableTorus->Create(m_Device.Get(), vertices, indices);
	GeometricPrimitive::CreateTeapot(vertices, indices, 3.0f, 32, false);
	m_DrawableTeapot->Create(m_Device.Get(), vertices, indices);

	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(VS_PS_ConstantBufferPerObject);

	D3D11_SUBRESOURCE_DATA cbData = {};
	cbData.pSysMem = &m_CbPerObjectData;

	if (FAILED(m_Device->CreateBuffer(&cbDesc, &cbData, m_CbPerObject.ReleaseAndGetAddressOf())))
		return false;

	cbDesc = {};
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.ByteWidth = sizeof(PS_ConstantBufferPerFrame);

	cbData = {};
	cbData.pSysMem = &m_CbPerFrameData;

	if (FAILED(m_Device->CreateBuffer(&cbDesc, &cbData, m_CbPerFrame.ReleaseAndGetAddressOf())))
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
	m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerAnisotropic.GetAddressOf());

	return true;
}

void DemoScene::UpdateScene(float dt)
{
	ImGui_NewFrame();

	XMVECTOR eyePos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
	//build projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 20.0f);
	
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR focus = XMVectorSet(0, 0, 0, 1);
	//build view matrix
	XMMATRIX view = XMMatrixLookAtLH(eyePos, focus, up);
	
	static float angle = 0.0f;
	angle += 45.0f * dt;
	
	XMMATRIX viewProj = view * proj;
	XMMATRIX RotateY = XMMatrixRotationY(XMConvertToRadians(angle));
	XMMATRIX RotateX = XMMatrixRotationX(XMConvertToRadians(angle));
	XMMATRIX RotateZ = XMMatrixRotationZ(XMConvertToRadians(angle));

	XMMATRIX worldBox = RotateY * XMMatrixTranslation(-2, -2, 0);
	XMMATRIX worldSphere = RotateY * XMMatrixTranslation(3.5f, -2, 0);
	XMMATRIX worldTorus = RotateZ * RotateX * XMMatrixTranslation(-2, 2, 2);
	XMMATRIX worldTeapot = RotateY * XMMatrixTranslation(4, 2, 2);

	XMMATRIX WVPCube = worldBox * viewProj;
	XMMATRIX WVPSphere = worldSphere * viewProj;
	XMMATRIX WVPTorus = worldTorus * viewProj;
	XMMATRIX WVPTeapot = worldTeapot * viewProj;

	XMStoreFloat4x4(&m_DrawableBox->WorldTransform, worldBox);
	XMStoreFloat4x4(&m_DrawableBox->WorldViewProjTransform, WVPCube);

	XMStoreFloat4x4(&m_DrawableSphere->WorldTransform, worldSphere);
	XMStoreFloat4x4(&m_DrawableSphere->WorldViewProjTransform, WVPSphere);
	XMStoreFloat4x4(&m_DrawableSphere->TextureTransform, XMMatrixScaling(2.0f, 2.0f, 0.0f));

	XMStoreFloat4x4(&m_DrawableTorus->WorldTransform, worldTorus);
	XMStoreFloat4x4(&m_DrawableTorus->WorldViewProjTransform, WVPTorus);

	XMStoreFloat4x4(&m_DrawableTeapot->WorldTransform, worldTeapot);
	XMStoreFloat4x4(&m_DrawableTeapot->WorldViewProjTransform, WVPTeapot);


	//DirLightVector is updated by ImGui widget
	static XMFLOAT3 DirLightVector = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//SetDirection method will normalize the vector before setting it
	m_CbPerFrameData.DirLight.SetDirection(DirLightVector);

	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePos);

	Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerFrame.Get(), &m_CbPerFrameData);

#pragma region ImGui Widgets
	if (!ImGui::Begin("Scene"))
	{
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Lights"))
	{
		if (ImGui::TreeNode("Directional Light"))
		{
			static bool isActive = true;
			static XMFLOAT3 oldDirection;
			static XMFLOAT4 oldAmbient;
			if (ImGui::Checkbox("Is Active##1", &isActive))
			{
				if (!isActive)
				{
					oldDirection = DirLightVector;
					oldAmbient = m_CbPerFrameData.DirLight.Ambient;

					//setting DirLightVector to zero-vector only zeros out diffuse & specular
					DirLightVector = XMFLOAT3(0.0f, 0.0f, 0.0f); 
					//we still need to zero out ambient manually
					m_CbPerFrameData.DirLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					m_CbPerFrameData.DirLight.Ambient = oldAmbient;
					DirLightVector = oldDirection;
				}
			}

			ImGui::ColorEdit4("Ambient##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Specular), 2);
			ImGui::InputFloat3("Direction##1", reinterpret_cast<float*>(&DirLightVector), 2);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Point Light"))
		{
			static bool isActive = true;
			static float oldValue;
			if (ImGui::Checkbox("Is Active##2", &isActive))
			{
				if (!isActive)
				{
					oldValue = m_CbPerFrameData.PointLight.Range;
					m_CbPerFrameData.PointLight.Range = 0;
				}
				else
					m_CbPerFrameData.PointLight.Range = oldValue;
			}

			ImGui::ColorEdit4("Ambient##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Specular), 2);
			ImGui::InputFloat3("Position##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Position), 2);
			ImGui::InputFloat("Range##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Range), 2);
			ImGui::InputFloat3("Attenuation##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Attenuation), 2);


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Spot Light"))
		{
			static bool isActive = true;
			static float oldValue;
			if (ImGui::Checkbox("Is Active##3", &isActive))
			{
				if (!isActive)
				{
					oldValue = m_CbPerFrameData.SpotLight.Range;
					m_CbPerFrameData.SpotLight.Range = 0;
				}
				else
					m_CbPerFrameData.SpotLight.Range = oldValue;
			}

			ImGui::ColorEdit4("Ambient##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Specular), 2);
			ImGui::InputFloat3("Position##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Position), 2);
			ImGui::InputFloat("Range##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Range), 2);
			ImGui::InputFloat("SpotPower", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.SpotPower), 2);
			ImGui::InputFloat3("Direction##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Direction), 2);
			ImGui::InputFloat3("Attenuation##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Attenuation), 2);

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


		if (ImGui::TreeNode("Torus"))
		{
			ImGui::InputFloat4("Ambient##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##5", reinterpret_cast<float*>(&m_DrawableTorus->Material.Specular), 2);

			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Teapot"))
		{
			ImGui::InputFloat4("Ambient##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##6", reinterpret_cast<float*>(&m_DrawableTeapot->Material.Specular), 2);

			ImGui::TreePop();
		}
	}

	ImGui::End();

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

	//Bind CbPerObject (slot: 0) to VS & PS
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	//Bind CbPerFrame (slot: 1) to PS
	m_ImmediateContext->PSSetConstantBuffers(1, 1, m_CbPerFrame.GetAddressOf());

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	static Drawable* drawables[] = { 
		m_DrawableTeapot.get(), m_DrawableTorus.get() , m_DrawableBox.get(), m_DrawableSphere.get() };

	for (auto const& object : drawables)
	{
		m_CbPerObjectData.WorldViewProj = object->WorldViewProjTransform;
		m_CbPerObjectData.World = object->WorldTransform;
		m_CbPerObjectData.TextureTransform = object->TextureTransform;
		m_CbPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(object->WorldTransform);
		m_CbPerObjectData.Material = object->Material;
		Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerObject.Get(), &m_CbPerObjectData);

		m_ImmediateContext->PSSetShaderResources(0, 1, object->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, object->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(object->IndexBuffer.Get(), object->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(object->IndexCount, 0, 0);

	}

//	ImGui::ShowDemoWindow();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_SwapChain->Present(1, 0);
}

