#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;

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
	m_DrawableGrid = std::make_unique<Drawable>();
	m_DrawableMirror = std::make_unique<Drawable>();

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
	m_DrawableMirror->Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
}

bool DemoScene::CreateDeviceDependentResources()
{
	if FAILED(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, GeometricPrimitive::VertexType::InputElementCount, g_VSBasicShader, sizeof(g_VSBasicShader), m_SimpleVertexLayout.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreateVertexShader(g_VSBasicShader, sizeof(g_VSBasicShader), nullptr, m_SimpleVertexShader.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(m_Device->CreatePixelShader(g_PSBasicShader, sizeof(g_PSBasicShader), nullptr, m_SimplePixelShader.ReleaseAndGetAddressOf()))
		return false;

#pragma region Load Textures
	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\WireFence.dds", 0, m_DrawableBox->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\checkboard.dds", 0, m_DrawableGrid->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\ice.dds", 0, m_DrawableMirror->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableSphere->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\rock.jpg", 0, m_DrawableTorus->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\flooring.png", 0, m_DrawableTeapot->TextureSRV.ReleaseAndGetAddressOf()))
		return false;
#pragma endregion

	CD3D11_DEFAULT d3dDefault;
	{
		CD3D11_SAMPLER_DESC desc(d3dDefault);
		desc.Filter = D3D11_FILTER_ANISOTROPIC;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MaxAnisotropy = 8;

		if FAILED(m_Device->CreateSamplerState(&desc, m_SamplerAnisotropic.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_BLEND_DESC desc(d3dDefault);
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSTransparent.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_RASTERIZER_DESC desc(d3dDefault);
		desc.CullMode = D3D11_CULL_NONE;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSCullNone.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_RASTERIZER_DESC(d3dDefault);
		desc.FillMode = D3D11_FILL_WIREFRAME;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSWireframe.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_RASTERIZER_DESC(d3dDefault);
		desc.FrontCounterClockwise = TRUE;

		if FAILED(m_Device->CreateRasterizerState(&desc, m_RSFrontCounterCW.ReleaseAndGetAddressOf()))
			return false;
	}
	{
		CD3D11_DEPTH_STENCIL_DESC desc(d3dDefault);
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSDisableWrite.ReleaseAndGetAddressOf()))
			return false;
	}

	CreateBuffers();

	return true;
}

void DemoScene::CreateBuffers()
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;
	
	GeometricPrimitive::CreateSphere(vertices, indices, 3.0f, 16, false);
	m_DrawableSphere->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(2.5f, 2.5f, 2.5f), false);
	m_DrawableBox->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateTorus(vertices, indices, 3.0f, 1.0f, 32, false);
	m_DrawableTorus->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateTeapot(vertices, indices, 3.0f, 8, false);
	m_DrawableTeapot->Create(m_Device.Get(), vertices, indices);

	GeometricPrimitive::CreateBox(vertices, indices, XMFLOAT3(0.1f, 5.0f, 20.0f), false);
	m_DrawableMirror->Create(m_Device.Get(), vertices, indices);

	Helpers::CreateGrid(vertices, indices, 100.0f, 100.0f);
	m_DrawableGrid->Create(m_Device.Get(), vertices, indices);

	Helpers::CreateConstantBuffer<PS_ConstantBufferPerFrame>(m_Device.Get(), m_CbPerFrame.ReleaseAndGetAddressOf());
	Helpers::CreateConstantBuffer<VS_PS_ConstantBufferPerObject>(m_Device.Get(), m_CbPerObject.ReleaseAndGetAddressOf());
}

bool DemoScene::Initialize()
{
	if (!Super::Initialize())
		return false;
	
	Super::ImGui_Init();

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
	Super::ImGui_NewFrame();

	//values updated by ImGui widgets
	static float cameraAngle = 0.0f;
	static XMVECTOR eyePos = XMVectorSet(0.0f, 7.0f, -10.0f, 1.0f);
	static XMVECTOR focus = XMVectorSet(0, 0, 0, 1);

	//build projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 100.0f);
	
	//build view matrix
	static XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX view = XMMatrixLookAtLH(XMVector3Transform(eyePos, XMMatrixRotationY(XMConvertToRadians(cameraAngle))), focus, up);
	
	static float angle = 0.0f;
	angle += 45.0f * dt;
	
	XMMATRIX viewProj = view * proj;
	XMMATRIX rotateY = XMMatrixRotationY(XMConvertToRadians(angle));
	XMMATRIX rotateX = XMMatrixRotationX(XMConvertToRadians(angle));
	XMMATRIX rotateZ = XMMatrixRotationZ(XMConvertToRadians(angle));

	XMMATRIX worldBox = rotateY * XMMatrixTranslation(-2, 2, 0);
	XMMATRIX worldTeapot = rotateY * XMMatrixTranslation(4, 2, 10);
	XMMATRIX worldSphere = rotateY * XMMatrixTranslation(3.5f, 2, 0);
	XMMATRIX worldTorus = rotateY * XMMatrixTranslation(-2, 1, 10);
	XMMATRIX worldMirror = XMMatrixTranslation(6.0f, 2.5f, 5.0f);

	XMStoreFloat4x4(&m_DrawableBox->WorldTransform, worldBox);
	XMStoreFloat4x4(&m_DrawableBox->ViewProjTransform, viewProj);

	XMStoreFloat4x4(&m_DrawableSphere->WorldTransform, worldSphere);
	XMStoreFloat4x4(&m_DrawableSphere->ViewProjTransform, viewProj);
	XMStoreFloat4x4(&m_DrawableSphere->TextureTransform, XMMatrixScaling(2.0f, 2.0f, 0.0f));

	XMStoreFloat4x4(&m_DrawableTorus->WorldTransform, worldTorus);
	XMStoreFloat4x4(&m_DrawableTorus->ViewProjTransform, viewProj);

	XMStoreFloat4x4(&m_DrawableTeapot->WorldTransform, worldTeapot);
	XMStoreFloat4x4(&m_DrawableTeapot->ViewProjTransform, viewProj);

	XMStoreFloat4x4(&m_DrawableGrid->ViewProjTransform, viewProj);
	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(10.0f, 10.0f, 0.0f));

	XMStoreFloat4x4(&m_DrawableMirror->WorldTransform, worldMirror);
	XMStoreFloat4x4(&m_DrawableMirror->ViewProjTransform, viewProj);
	XMStoreFloat4x4(&m_DrawableMirror->TextureTransform, XMMatrixScaling(2.0f, 1.0f, 0.0f));

	//DirLightVector is updated by ImGui widget
	static XMFLOAT3 dirLightVector = XMFLOAT3(1.0f, 0.0f, 0.0f);
	//SetDirection method will normalize the vector before setting it
	m_CbPerFrameData.DirLight.SetDirection(dirLightVector);

	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePos);

	Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerFrame.Get(), &m_CbPerFrameData);

#pragma region ImGui Widgets
	if (!ImGui::Begin("Scene"))
	{
		ImGui::End();
		return;
	}

	if (ImGui::CollapsingHeader("Screen Resolution"))
	{
		//input of client size area
		static int inputWidth = m_ClientWidth;
		static int inputHeight = m_ClientHeight;

		ImGui::InputInt("Width", &inputWidth, 100, 200);
		ImGui::InputInt("Height", &inputHeight, 100, 200);
		
		if (ImGui::Button("Apply"))
		{
			//Should probably clamp it on min/max values but works for now
			if (inputWidth < MIN_WIDTH || inputHeight < MIN_HEIGHT ||
				inputWidth > MAX_WIDTH || inputHeight > MAX_HEIGHT)
			{
				inputWidth = MIN_WIDTH;
				inputHeight = MIN_HEIGHT;
			}
			
			if (inputWidth == m_ClientWidth && inputHeight == m_ClientHeight)
				return;

			//Calculate window size from client size
			RECT wr = { 0, 0, inputWidth, inputHeight };
			AdjustWindowRect(&wr, g_WindowStyle, 0);

			UINT windowWidth = static_cast<UINT>(wr.right - wr.left);
			UINT windowHeight = static_cast<UINT>(wr.bottom - wr.top);
			
			//Resize window area
			assert(SetWindowPos(m_MainWindow, 0, 0, 0, windowWidth, windowHeight,
				SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER) != 0);
			
			//Resize color & depth buffers
			Super::OnResize();

		}
	}

	if (ImGui::CollapsingHeader("Camera"))
	{
		ImGui::DragFloat3("EyePos", reinterpret_cast<float*>(&eyePos), 1.0f, -100.0f, 100.0f, "%.2f");
		ImGui::DragFloat3("Focus", reinterpret_cast<float*>(&focus), 1.0f, -100.0f, 100.0f), "%.2f";
		
		if (XMVector3Equal(eyePos, XMVectorZero()))
			eyePos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

		ImGui::SliderFloat("Rotate-Y", &cameraAngle, 0.0f, 360.0f);
	}

	if (ImGui::CollapsingHeader("Effects"))
	{
		if (ImGui::TreeNode("Fog"))
		{
			static bool isActive = true;
			if (ImGui::Checkbox("Enabled", &isActive))
			{
				if (isActive)
					m_CbPerFrameData.Fog.FogEnabled = 1.0f;
				else
					m_CbPerFrameData.Fog.FogEnabled = 0.0f;
			}

			ImGui::InputFloat("Fog Start", &m_CbPerFrameData.Fog.FogStart);
			ImGui::InputFloat("Fog Range", &m_CbPerFrameData.Fog.FogRange);
			ImGui::ColorEdit3("Fog Color", reinterpret_cast<float*>(&m_CbPerFrameData.Fog.FogColor));

			ImGui::TreePop();
		}
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
					oldDirection = dirLightVector;
					oldAmbient = m_CbPerFrameData.DirLight.Ambient;

					//setting DirLightVector to zero-vector only zeros out diffuse & specular color
					dirLightVector = XMFLOAT3(0.0f, 0.0f, 0.0f); 
					//we still need to zero out ambient color manually
					m_CbPerFrameData.DirLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					m_CbPerFrameData.DirLight.Ambient = oldAmbient;
					dirLightVector = oldDirection;
				}
			}

			ImGui::ColorEdit4("Ambient##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##1", reinterpret_cast<float*>(&m_CbPerFrameData.DirLight.Specular), 2);
			ImGui::InputFloat3("Direction##1", reinterpret_cast<float*>(&dirLightVector), 2);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Point Light"))
		{
			static bool isActive = true;
			static float oldRange;
			if (ImGui::Checkbox("Is Active##2", &isActive))
			{
				if (!isActive)
				{
					oldRange = m_CbPerFrameData.PointLight.Range;
					m_CbPerFrameData.PointLight.Range = 0;
				}
				else
					m_CbPerFrameData.PointLight.Range = oldRange;
			}

			ImGui::ColorEdit4("Ambient##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##2", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Specular), 2);
			ImGui::InputFloat3("Position##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Position), 2);
			ImGui::InputFloat("Range##1", &m_CbPerFrameData.PointLight.Range, 2);
			ImGui::InputFloat3("Attenuation##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Attenuation), 2);


			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Spot Light"))
		{
			static bool isActive = true;
			static float oldRange;
			if (ImGui::Checkbox("Is Active##3", &isActive))
			{
				if (!isActive)
				{
					oldRange = m_CbPerFrameData.SpotLight.Range;
					m_CbPerFrameData.SpotLight.Range = 0;
				}
				else
					m_CbPerFrameData.SpotLight.Range = oldRange;
			}

			ImGui::ColorEdit4("Ambient##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##5", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Specular), 2);
			ImGui::InputFloat3("Position##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Position), 2);
			ImGui::InputFloat("Range##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Range), 2);
			ImGui::InputFloat("SpotPower", &m_CbPerFrameData.SpotLight.SpotPower, 2);
			ImGui::InputFloat3("Direction##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Direction), 2);
			ImGui::InputFloat3("Attenuation##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Attenuation), 2);

			ImGui::TreePop();
		}
	}

	if (ImGui::CollapsingHeader("Object Materials"))
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
	{
		static bool bWireframe = false;
		if (ImGui::CollapsingHeader("Render States"))
		{
			ImGui::Checkbox("Wireframe", &bWireframe);
		}

		if (bWireframe)
			m_ImmediateContext->RSSetState(m_RSWireframe.Get());
		else
			m_ImmediateContext->RSSetState(nullptr);
	}
	ImGui::End();

#pragma endregion
}

void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::Silver;
	ImGui::PushItemWidth(ImGui::GetColumnWidth() * 0.5f);
	ImGui::ColorEdit4("clear color", reinterpret_cast<float*>(&backBufferColor));
	ImGui::PopItemWidth();

	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<float*>(&backBufferColor));
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

	static Drawable* drawables[] = { m_DrawableGrid.get(), m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };

	for (auto const& it : drawables)
	{
		m_CbPerObjectData.WorldViewProj = it->GetWorldViewProj();
		m_CbPerObjectData.World = it->WorldTransform;
		m_CbPerObjectData.TextureTransform = it->TextureTransform;
		m_CbPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(it->WorldTransform);
		m_CbPerObjectData.Material = it->Material;

		Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerObject.Get(), &m_CbPerObjectData);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//======= reflect across the plane =======//

	static Drawable* reflectedDrawables[] = { m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };

	XMVECTOR plane = XMPlaneFromPointNormal(XMVectorSet(6.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
	
	m_ImmediateContext->RSSetState(m_RSFrontCounterCW.Get());
	for (auto const& it : reflectedDrawables)
	{
		XMMATRIX reflectedWorld = XMLoadFloat4x4(&it->WorldTransform) * XMMatrixReflect(plane);
		XMMATRIX reflectedWVP = reflectedWorld * XMLoadFloat4x4(&it->ViewProjTransform);

		XMStoreFloat4x4(&m_CbPerObjectData.World, reflectedWorld);
		XMStoreFloat4x4(&m_CbPerObjectData.WorldViewProj, reflectedWVP);
		XMStoreFloat4x4(&m_CbPerObjectData.WorldInvTranspose, Helpers::ComputeInverseTranspose(reflectedWorld));

		m_CbPerObjectData.TextureTransform = it->TextureTransform;
		m_CbPerObjectData.Material = it->Material;

		Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerObject.Get(), &m_CbPerObjectData);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}
	m_ImmediateContext->RSSetState(nullptr);
	//=====================================================//

	static Drawable* transparentDrawables[] = {   m_DrawableMirror.get(), m_DrawableBox.get() };
	
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_ImmediateContext->OMSetBlendState(m_BSTransparent.Get(), blendFactor, 0xffffffff);
	m_ImmediateContext->RSSetState(m_RSCullNone.Get());
	m_ImmediateContext->OMSetDepthStencilState(m_DSSDisableWrite.Get(), 0);

	for (auto const& it : transparentDrawables)
	{
		m_CbPerObjectData.WorldViewProj = it->GetWorldViewProj();
		m_CbPerObjectData.World = it->WorldTransform;
		m_CbPerObjectData.TextureTransform = it->TextureTransform;
		m_CbPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(it->WorldTransform);
		m_CbPerObjectData.Material = it->Material;

		Helpers::UpdateConstantBuffer(m_ImmediateContext.Get(), m_CbPerObject.Get(), &m_CbPerObjectData);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	m_ImmediateContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	m_ImmediateContext->RSSetState(nullptr);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);

	Present();
}

void DemoScene::Present()
{
	static bool bVSync = true;
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Checkbox("VSync:", &bVSync);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	
		ImGui::EndMainMenuBar();
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	if (bVSync)
		m_SwapChain->Present(1, 0);
	else
		m_SwapChain->Present(0, 0);
}

