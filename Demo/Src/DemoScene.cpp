#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Drawable.h"

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;

//These headers may not be present, so you may need to build the project once
namespace
{
	#include "Shaders\Compiled\BasicPS.h"
	#include "Shaders\Compiled\BasicVS.h"
	#include "Shaders\Compiled\SimplePS.h"
	#include "Shaders\Compiled\SimpleVS.h"
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
	m_CbPerFrameData.DirLight.Direction = XMFLOAT3(1.0f, 1.0f, 1.0f);
	//Setup Pointlight
	m_CbPerFrameData.PointLight.Position = XMFLOAT3(0.0f, 2.0f, -2.0f);
	//Setup Spotlight
	m_CbPerFrameData.SpotLight.Position = XMFLOAT3(-6.0f, 4.0f, 6.0f);
	m_CbPerFrameData.SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_CbPerFrameData.SpotLight.SpotPower = 2.0f;

	//Setup some material properties other than default
	m_DrawableBox->Material.Ambient = XMFLOAT4(0.35f, 0.35f, 0.35f, 1.0f);
	m_DrawableSphere->Material.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_DrawableSphere->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 64.0f);
	m_DrawableTorus->Material.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	m_DrawableTeapot->Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 32.0f);
	m_DrawableMirror->Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.3f);
	m_DrawableGrid->Material.Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_DrawableGrid->Material.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
}

bool DemoScene::CreateDeviceDependentResources()
{
	m_CbPerFrame.Create(m_Device.Get());
	m_CbPerObject.Create(m_Device.Get());
	m_CbConstants.Create(m_Device.Get());

	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosNormalTex;
	DX::ThrowIfFailed(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, GeometricPrimitive::VertexType::InputElementCount,
		g_BasicVS, sizeof(g_BasicVS), layoutPosNormalTex.ReleaseAndGetAddressOf()));

	m_BasicEffect.Create(m_Device.Get(), layoutPosNormalTex, g_BasicVS, sizeof(g_BasicVS), g_BasicPS, sizeof(g_BasicPS));
	m_SimpleEffect.Create(m_Device.Get(), layoutPosNormalTex, g_SimpleVS, sizeof(g_SimpleVS), g_SimplePS, sizeof(g_SimplePS));

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

		desc = CD3D11_BLEND_DESC(d3dDefault);
		desc.RenderTarget[0].RenderTargetWriteMask = NULL;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSNoColorWrite.ReleaseAndGetAddressOf()))
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

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSNoDepthWrite.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSMarkPixels.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSDrawMarkedOnly.ReleaseAndGetAddressOf()))
			return false;

		desc = CD3D11_DEPTH_STENCIL_DESC(d3dDefault);
		desc.StencilEnable = TRUE;
		desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR_SAT;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
		desc.BackFace = desc.FrontFace;

		if FAILED(m_Device->CreateDepthStencilState(&desc, m_DSSNoDoubleBlend.ReleaseAndGetAddressOf()))
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

	Helpers::CreateGrid(vertices, indices, 100, 100);
	m_DrawableGrid->Create(m_Device.Get(), vertices, indices);

}

bool DemoScene::Initialize()
{
	if (!Super::Initialize())
		return false;
	
	if (!CreateDeviceDependentResources())
		return false;

	Super::ImGui_Init();

	return true;
}

void DemoScene::UpdateScene(float dt)
{
	Super::ImGui_NewFrame();

	//values updated by ImGui widgets
	static float cameraAngle = 0.0f;
	static XMVECTOR initEyePos = XMVectorSet(0.0f, 7.0f, -10.0f, 1.0f);
	static XMVECTOR focus = XMVectorSet(0, 0, 0, 1);

	//build projection matrix
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 100.0f);
	XMStoreFloat4x4(&m_CameraProjection, proj);

	//build view matrix
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR eyePosition = XMVector3Transform(initEyePos, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));
	XMMATRIX view = XMMatrixLookAtLH(eyePosition, focus, up);
	XMStoreFloat4x4(&m_CameraView, view);

	static float angle = 0.0f;
	angle += 45.0f * dt;
	
	XMMATRIX viewProj = view * proj;
	XMMATRIX rotateY = XMMatrixRotationY(XMConvertToRadians(angle));
	XMMATRIX rotateX = XMMatrixRotationX(XMConvertToRadians(angle));
	XMMATRIX rotateZ = XMMatrixRotationZ(XMConvertToRadians(angle));

	m_DrawableBox->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-2, 2, 0));
	m_DrawableTeapot->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(2, 2, 10));
	m_DrawableSphere->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(3, 2, 0));
	m_DrawableTorus->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-4, 2, 6));
	m_DrawableMirror->WorldTransform = Helpers::XMMatrixToStorage(XMMatrixTranslation(6.0f, 2.5f, 5.0f));

	XMStoreFloat4x4(&m_DrawableSphere->TextureTransform, XMMatrixScaling(2.0f, 2.0f, 0.0f));
	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(20.0f, 20.0f, 0.0f));
	XMStoreFloat4x4(&m_DrawableMirror->TextureTransform, XMMatrixScaling(2.0f, 1.0f, 0.0f));

	//DirLightVector is updated by ImGui widget
	static XMFLOAT3 dirLightVector = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	
	//SetDirection method will normalize the vector before setting it
	m_CbPerFrameData.DirLight.SetDirection(dirLightVector);

	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePosition);

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
		ImGui::DragFloat3("EyePos", reinterpret_cast<float*>(&initEyePos), 1.0f, -100.0f, 100.0f, "%.2f");
		ImGui::DragFloat3("Focus", reinterpret_cast<float*>(&focus), 1.0f, -100.0f, 100.0f), "%.2f";
		
		if (XMVector3Equal(initEyePos, XMVectorZero()))
			initEyePos = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);

		ImGui::SliderFloat("Rotate-Y", &cameraAngle, 0.0f, 360.0f);
	}

	if (ImGui::CollapsingHeader("Effects"))
	{
		if (ImGui::TreeNode("Fog"))
		{
			static bool isActive = false;
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
			ImGui::DragFloat3("Position##1", reinterpret_cast<float*>(&m_CbPerFrameData.PointLight.Position));
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
			ImGui::DragFloat3("Position##2", reinterpret_cast<float*>(&m_CbPerFrameData.SpotLight.Position));
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

		if (ImGui::TreeNode("Grid"))
		{
			ImGui::InputFloat4("Ambient##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Ambient), 2);
			ImGui::InputFloat4("Diffuse##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Diffuse), 2);
			ImGui::InputFloat4("Specular##6", reinterpret_cast<float*>(&m_DrawableGrid->Material.Specular), 2);

			ImGui::TreePop();
		}
	}
	ImGui::End();

#pragma endregion
}


void DemoScene::Clear()
{
	static XMVECTOR backBufferColor = DirectX::Colors::Black;
	ImGui::PushItemWidth(ImGui::GetColumnWidth() * 0.5f);
	ImGui::ColorEdit4("clear color", reinterpret_cast<float*>(&backBufferColor));
	ImGui::PopItemWidth();

	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), reinterpret_cast<float*>(&backBufferColor));
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DemoScene::PrepareForRendering()
{
	m_BasicEffect.Bind(m_ImmediateContext.Get());

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_ImmediateContext->PSSetSamplers(0, 1, m_SamplerAnisotropic.GetAddressOf());
	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(1, 1, m_CbPerFrame.GetAddressOf());

	m_CbPerFrame.SetData(m_ImmediateContext.Get(), m_CbPerFrameData);
}


void DemoScene::DrawScene()
{
	Clear();
	PrepareForRendering();

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	static Drawable* drawables[] = { m_DrawableGrid.get(), m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };
	for (auto const& it : drawables)
	{
		FillPerObjectConstantBuffer(it);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//======================================== planar shadows ==============================================================//
	static Drawable* drawablesShadow[] = { m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };

	m_SimpleEffect.Bind(m_ImmediateContext.Get());

	m_ImmediateContext->VSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());
	m_ImmediateContext->PSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());

	m_ImmediateContext->OMSetBlendState(m_BSTransparent.Get(), NULL, 0xffffffff);
	m_ImmediateContext->OMSetDepthStencilState(m_DSSNoDoubleBlend.Get(), 0);

	XMVECTOR shadowPlane = XMPlaneFromPointNormal(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX shadowMatrix = XMMatrixShadow(shadowPlane, -XMLoadFloat3(&m_CbPerFrameData.DirLight.Direction));
	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);

	for (auto const& it : drawablesShadow)
	{
		XMMATRIX world = it->GetWorld() * shadowMatrix * XMMatrixTranslation(0.0f, 0.001f, 0.0f);
		
		m_CbConstantsData.Color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.4f); 
		m_CbConstantsData.WorldViewProj = Helpers::XMMatrixToStorage(world * viewProj);

		m_CbConstants.SetData(m_ImmediateContext.Get(), m_CbConstantsData);

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//rebind relevant shaders, constant buffers and input layout
	PrepareForRendering();

	//==================================================================================================================//


	//Mark mirror pixels in stencil buffer to 1
	RenderToStencil();
	//Draw reflections only where stencil buffer value is 1
	RenderReflections();
	
	//draw transparent objects last
	static Drawable* transparentDrawables[] = {   m_DrawableMirror.get() /*, m_DrawableBox.get()*/ };
	
	m_ImmediateContext->OMSetBlendState(m_BSTransparent.Get(), NULL, 0xffffffff);
	m_ImmediateContext->RSSetState(m_RSCullNone.Get());
	m_ImmediateContext->OMSetDepthStencilState(m_DSSNoDepthWrite.Get(), 0);

	for (auto const& it : transparentDrawables)
	{
		FillPerObjectConstantBuffer(it);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}
	
	ResetStates();
	Present();
}

void DemoScene::RenderToStencil()
{

	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	//don't write to backbuffer while marking mirror pixels in stencilbuffer 
	m_ImmediateContext->OMSetBlendState(m_BSNoColorWrite.Get(), NULL, 0xffffffff);
	//disables depth write as well
	m_ImmediateContext->OMSetDepthStencilState(m_DSSMarkPixels.Get(), 1);

	FillPerObjectConstantBuffer(m_DrawableMirror.get());

	m_ImmediateContext->PSSetShaderResources(0, 1, m_DrawableMirror->TextureSRV.GetAddressOf());
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_DrawableMirror->VertexBuffer.GetAddressOf(), &stride, &offset);
	m_ImmediateContext->IASetIndexBuffer(m_DrawableMirror->IndexBuffer.Get(), m_DrawableMirror->IndexBufferFormat, 0);
	m_ImmediateContext->DrawIndexed(m_DrawableMirror->IndexCount, 0, 0);

	//reset blend state so color write gets enabled again
	m_ImmediateContext->OMSetBlendState(nullptr, NULL, 0xffffffff);

	//not resetting depth state because other render pass after this ( RenderReflections() ) is explicitly setting it
}

void DemoScene::RenderReflections()
{
	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	XMVECTOR reflectPlane = XMPlaneFromPointNormal(XMVectorSet(6.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
	XMMATRIX reflectionMatrix = XMMatrixReflect(reflectPlane);

	//Reflect lights 
	PS_CbPerFrame_BasicShader reflectedCbPerFrameData = m_CbPerFrameData;
	reflectedCbPerFrameData.DirLight = Helpers::GetReflectedLight(m_CbPerFrameData.DirLight, reflectionMatrix);
	reflectedCbPerFrameData.PointLight = Helpers::GetReflectedLight(m_CbPerFrameData.PointLight, reflectionMatrix);
	reflectedCbPerFrameData.SpotLight = Helpers::GetReflectedLight(m_CbPerFrameData.SpotLight, reflectionMatrix);
	
	m_CbPerFrame.SetData(m_ImmediateContext.Get(), reflectedCbPerFrameData);

	//Draw reflection only through the mirror i.e where stencil value = 1
	m_ImmediateContext->OMSetDepthStencilState(m_DSSDrawMarkedOnly.Get(), 1);

	//reflected triangles' winding order gets reversed, so setting counter clock wise as front facing
	m_ImmediateContext->RSSetState(m_RSFrontCounterCW.Get());

	//Now draw reflected objects
	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);
	static Drawable* reflectedDrawables[] = { m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };

	for (auto const& it : reflectedDrawables)
	{
		XMMATRIX reflectedWorld = it->GetWorld() * reflectionMatrix;

		m_CbPerObjectData.World = Helpers::XMMatrixToStorage(reflectedWorld);
		m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(reflectedWorld * viewProj);
		m_CbPerObjectData.WorldInvTranspose = Helpers::XMMatrixToStorage(Helpers::ComputeInverseTranspose(reflectedWorld));
		m_CbPerObjectData.TextureTransform = it->TextureTransform;
		m_CbPerObjectData.Material = it->Material;

		m_CbPerObject.SetData(m_ImmediateContext.Get(), m_CbPerObjectData);

		m_ImmediateContext->PSSetShaderResources(0, 1, it->TextureSRV.GetAddressOf());
		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//Restore lights to their old positions/directions
	m_CbPerFrame.SetData(m_ImmediateContext.Get(), m_CbPerFrameData);
}

void DemoScene::FillPerObjectConstantBuffer(Drawable* const drawable)
{
	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);

	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(drawable->GetWorld() * viewProj);
	m_CbPerObjectData.World = drawable->WorldTransform;
	m_CbPerObjectData.TextureTransform = drawable->TextureTransform;
	m_CbPerObjectData.WorldInvTranspose = Helpers::ComputeInverseTranspose(drawable->WorldTransform);
	m_CbPerObjectData.Material = drawable->Material;

	m_CbPerObject.SetData(m_ImmediateContext.Get(), m_CbPerObjectData);
}

void DemoScene::ResetStates()
{
	static bool bWireframe = false;
	ImGui::Checkbox("Wireframe", &bWireframe);
	
	if (bWireframe)
		m_ImmediateContext->RSSetState(m_RSWireframe.Get());
	else
		m_ImmediateContext->RSSetState(nullptr);

	m_ImmediateContext->OMSetBlendState(nullptr, NULL, 0xffffffff);
	m_ImmediateContext->OMSetDepthStencilState(nullptr, 0);
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

