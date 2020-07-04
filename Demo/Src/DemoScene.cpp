#include "pch.h"
#include "DemoScene.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>
#include <GeometricPrimitive.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "Drawable.h"
#include "Vertex.h"

//Used in AdjustWindowRect(..) while resizing 
extern DWORD g_WindowStyle;

//These headers may not be present, so you may need to build the project once
namespace
{
	#include "Shaders\Compiled\BasicVS.h"
	#include "Shaders\Compiled\BillboardVS.h"
	#include "Shaders\Compiled\SimpleComputeCS.h"
}

using namespace DirectX;

DemoScene::DemoScene(const HWND& hwnd) :
	Super(hwnd)
{
	m_DrawableBox = std::make_unique<Drawable>();
	m_DrawableSphere = std::make_unique<Drawable>();
	m_DrawableTorus = std::make_unique<Drawable>();
	m_DrawableTeapot = std::make_unique<Drawable>();
	m_DrawableGrid = std::make_unique<Drawable>();
	m_DrawableMirror = std::make_unique<Drawable>();

	//Setup DirectionalLight
	m_DirLight.SetDirection(XMFLOAT3(-1.0f, -1.0f, 0.0f));
	//Setup Pointlight
	m_PointLight.Position = XMFLOAT3(0.0f, 2.0f, -2.0f);
	//Setup Spotlight
	m_SpotLight.Position = XMFLOAT3(-6.0f, 4.0f, 6.0f);
	m_SpotLight.Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_SpotLight.SpotPower = 2.0f;

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
	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPosNormalTex;
	DX::ThrowIfFailed(m_Device->CreateInputLayout(GeometricPrimitive::VertexType::InputElements, GeometricPrimitive::VertexType::InputElementCount, g_BasicVS, sizeof(g_BasicVS), layoutPosNormalTex.ReleaseAndGetAddressOf()));

	m_BasicEffect.Create(m_Device.Get(), layoutPosNormalTex);
	m_SimpleEffect.Create(m_Device.Get(), layoutPosNormalTex);

	Microsoft::WRL::ComPtr<ID3D11InputLayout> layoutPointSize;
	DX::ThrowIfFailed(m_Device->CreateInputLayout(TreePointSprite::InputElements, TreePointSprite::ElementCount, g_BillboardVS, sizeof(g_BillboardVS), layoutPointSize.ReleaseAndGetAddressOf()));

	m_BillboardEffect.Create(m_Device.Get(), layoutPointSize);
	
#pragma region Load Textures
	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\crate.dds", 0, m_DrawableBox->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\mipmaps.dds", 0, m_DrawableGrid->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\ice.dds", 0, m_DrawableMirror->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\metal.jpg", 0, m_DrawableSphere->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\rock.jpg", 0, m_DrawableTorus->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateWICTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\flooring.png", 0, m_DrawableTeapot->TextureSRV.ReleaseAndGetAddressOf()))
		return false;

	if FAILED(CreateDDSTextureFromFile(m_Device.Get(), m_ImmediateContext.Get(), L"Textures\\treeArray.dds", 0, m_TreeTexArraySRV.ReleaseAndGetAddressOf()))
		return false;
#pragma endregion

#pragma region States Creation
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

		desc = CD3D11_BLEND_DESC(d3dDefault);
		desc.AlphaToCoverageEnable = TRUE;

		if FAILED(m_Device->CreateBlendState(&desc, m_BSAlphaToCoverage.ReleaseAndGetAddressOf()))
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
#pragma endregion

	CreateBuffers();
	CreateComputeShaderResources();

	return true;
}

void DemoScene::CreateComputeShaderResources()
{
	DX::ThrowIfFailed(m_Device->CreateComputeShader(g_SimpleCompute, sizeof(g_SimpleCompute), nullptr, m_SimpleComputeShader.ReleaseAndGetAddressOf()));

	std::vector<ComputeData> data(256);

	for (int i = 0; i < 256; ++i)
	{
		data[i].v1 = XMFLOAT4(i, i, i, i);
		data[i].v2 = XMFLOAT3(i*2, i*2, i*2);
	}

	ID3D11Buffer* input;
	Helpers::CreateStructuredBuffer(m_Device.Get(), data, D3D11_BIND_UNORDERED_ACCESS, &input);

	Helpers::CreateStructuredBuffer<ComputeData>(m_Device.Get(), 256, D3D11_BIND_UNORDERED_ACCESS, m_OutputBuffer.ReleaseAndGetAddressOf());
	Helpers::CreateStructuredBuffer<ComputeData>(m_Device.Get(), 256, 0, m_OutputSystemBuffer.ReleaseAndGetAddressOf(), D3D11_CPU_ACCESS_READ, D3D11_USAGE_STAGING);

	CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = CD3D11_UNORDERED_ACCESS_VIEW_DESC(input, DXGI_FORMAT_UNKNOWN, 0, 256, D3D11_BUFFER_UAV_FLAG_APPEND);
	DX::ThrowIfFailed(m_Device->CreateUnorderedAccessView(input, &uavDesc, m_InputUAV.ReleaseAndGetAddressOf()));
	DX::ThrowIfFailed(m_Device->CreateUnorderedAccessView(m_OutputBuffer.Get(), &uavDesc, m_OutputUAV.ReleaseAndGetAddressOf()));

	input->Release();
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


	std::array<TreePointSprite, 6> points = 
	{
		TreePointSprite(XMFLOAT3(-5.0f, 5.0f, 10.0f), XMFLOAT2(5.0f, 10.0f)),
		TreePointSprite(XMFLOAT3(-10.0f, 5.0f, 10.0f), XMFLOAT2(5.0f, 10.0f)),
		TreePointSprite(XMFLOAT3(-10.0f, 5.0f, 15.0f), XMFLOAT2(5.0f, 10.0f)),
		TreePointSprite(XMFLOAT3(-10.0f, 5.0f, 20.0f), XMFLOAT2(5.0f, 10.0f)),
		TreePointSprite(XMFLOAT3(-20.0f, 5.0f, 20.0f), XMFLOAT2(5.0f, 10.0f)),
		TreePointSprite(XMFLOAT3(-30.0f, 5.0f, 20.0f), XMFLOAT2(5.0f, 10.0f))
	};

	Helpers::CreateMeshBuffer(m_Device.Get(), points.data(), points.size(), D3D11_BIND_VERTEX_BUFFER, m_TreePointsVB.ReleaseAndGetAddressOf());
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
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(60.0f), static_cast<float>(m_ClientWidth) / m_ClientHeight, 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_CameraProjection, proj);

	//build view matrix
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR eyePosition = XMVector3Transform(initEyePos, XMMatrixRotationY(XMConvertToRadians(cameraAngle)));
	XMMATRIX view = XMMatrixLookAtLH(eyePosition, focus, up);
	XMStoreFloat4x4(&m_CameraView, view);

	static float angle = 0.0f;
	angle += 45.0f * dt;
	
	XMMATRIX rotateY = XMMatrixRotationY(XMConvertToRadians(angle));

	m_DrawableBox->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-2, 2, 0));
	m_DrawableTeapot->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(2, 2, 10));
	m_DrawableSphere->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(3, 2, 0));
	m_DrawableTorus->WorldTransform = Helpers::XMMatrixToStorage(rotateY * XMMatrixTranslation(-4, 2, 6));
	m_DrawableMirror->WorldTransform = Helpers::XMMatrixToStorage(XMMatrixTranslation(6.0f, 2.5f, 5.0f));

	XMStoreFloat4x4(&m_DrawableSphere->TextureTransform, XMMatrixScaling(2.0f, 2.0f, 0.0f));
	XMStoreFloat4x4(&m_DrawableGrid->TextureTransform, XMMatrixScaling(20.0f, 20.0f, 0.0f));
	XMStoreFloat4x4(&m_DrawableMirror->TextureTransform, XMMatrixScaling(2.0f, 1.0f, 0.0f));


	m_BasicEffect.SetEyePosition(eyePosition);
	m_BasicEffect.SetFog(m_Fog);
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

	m_BillboardEffect.SetEyePos(eyePosition);
	m_BillboardEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

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
					m_Fog.FogEnabled = 1.0f;
				else
					m_Fog.FogEnabled = 0.0f;
			}

			ImGui::InputFloat("Fog Start", &m_Fog.FogStart);
			ImGui::InputFloat("Fog Range", &m_Fog.FogRange);
			ImGui::ColorEdit3("Fog Color", reinterpret_cast<float*>(&m_Fog.FogColor));

			ImGui::TreePop();
		}
	}

	static XMFLOAT3 dirLightVec = m_DirLight.Direction;
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
					oldDirection = dirLightVec;
					oldAmbient = m_DirLight.Ambient;

					//setting DirLightVector to zero-vector only zeros out diffuse & specular color
					dirLightVec = XMFLOAT3(0.0f, 0.0f, 0.0f);
					//we still need to zero out ambient color manually
					m_DirLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
				}
				else
				{
					m_DirLight.Ambient = oldAmbient;
					dirLightVec = oldDirection;
				}
			}

			ImGui::ColorEdit4("Ambient##1", reinterpret_cast<float*>(&m_DirLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##1", reinterpret_cast<float*>(&m_DirLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##1", reinterpret_cast<float*>(&m_DirLight.Specular), 2);
			
			ImGui::InputFloat3("Direction##1", reinterpret_cast<float*>(&dirLightVec), 2);
			m_DirLight.SetDirection(dirLightVec);

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
					oldRange = m_PointLight.Range;
					m_PointLight.Range = 0;
				}
				else
					m_PointLight.Range = oldRange;
			}

			ImGui::ColorEdit4("Ambient##2", reinterpret_cast<float*>(&m_PointLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##2", reinterpret_cast<float*>(&m_PointLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##2", reinterpret_cast<float*>(&m_PointLight.Specular), 2);
			ImGui::DragFloat3("Position##1", reinterpret_cast<float*>(&m_PointLight.Position));
			ImGui::InputFloat("Range##1", &m_PointLight.Range, 2);
			ImGui::InputFloat3("Attenuation##1", reinterpret_cast<float*>(&m_PointLight.Attenuation), 2);


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
					oldRange = m_SpotLight.Range;
					m_SpotLight.Range = 0;
				}
				else
					m_SpotLight.Range = oldRange;
			}

			ImGui::ColorEdit4("Ambient##5", reinterpret_cast<float*>(&m_SpotLight.Ambient), 2);
			ImGui::ColorEdit4("Diffuse##5", reinterpret_cast<float*>(&m_SpotLight.Diffuse), 2);
			ImGui::ColorEdit3("Specular##5", reinterpret_cast<float*>(&m_SpotLight.Specular), 2);
			ImGui::DragFloat3("Position##2", reinterpret_cast<float*>(&m_SpotLight.Position));
			ImGui::InputFloat("Range##2", reinterpret_cast<float*>(&m_SpotLight.Range), 2);
			ImGui::InputFloat("SpotPower", &m_SpotLight.SpotPower, 2);
			ImGui::InputFloat3("Direction##2", reinterpret_cast<float*>(&m_SpotLight.Direction), 2);
			ImGui::InputFloat3("Attenuation##2", reinterpret_cast<float*>(&m_SpotLight.Attenuation), 2);

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

	ImGui::Text("deltaTime: %f", dt);

	ImGui::End();

#pragma endregion
}

void DemoScene::TestComputeShader()
{
	if (!ImGui::Begin("Compute Shader"))
	{
		ImGui::End();
		return;
	}
	
	static ComputeData* data = nullptr;
	if (ImGui::Button("Dispatch"))
	{

		ID3D11UnorderedAccessView* uavs[] = { m_InputUAV.Get(), m_OutputUAV.Get() };
		ID3D11UnorderedAccessView* uavNull[] = { nullptr };
		UINT counts[] = { 256, 0 };

		m_ImmediateContext->CSSetShader(m_SimpleComputeShader.Get(), nullptr, 0);
		m_ImmediateContext->CSSetUnorderedAccessViews(0, 2, uavs, counts);

		m_ImmediateContext->Dispatch(1, 1, 1);

		m_ImmediateContext->CSSetUnorderedAccessViews(0, 1, uavNull, 0);
		m_ImmediateContext->CopyResource(m_OutputSystemBuffer.Get(), m_OutputBuffer.Get());
		m_ImmediateContext->CSSetShader(nullptr, nullptr, 0);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		m_ImmediateContext->Map(m_OutputSystemBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedRes);
		data = reinterpret_cast<ComputeData*>(mappedRes.pData);
		m_ImmediateContext->Unmap(m_OutputSystemBuffer.Get(), 0);

	}

	if (data != nullptr)
	{
		for (UINT i = 0; i < 256; ++i)
		{
			ImGui::Text("index: %d \
			v1: %.2f %.2f %.2f %.2f \
			v2: %.2f %.2f %.2f",
				i,
				data[i].v1.x, data[i].v1.y, data[i].v1.z, data[i].v1.w,
				data[i].v2.x, data[i].v2.y, data[i].v2.z);
		}
	}

	ImGui::End();
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
	m_BasicEffect.SetSampler(m_ImmediateContext.Get(), m_SamplerAnisotropic.Get());

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}


void DemoScene::DrawScene()
{
	TestComputeShader();
	Clear();
	PrepareForRendering();

	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);
	UINT stride = sizeof(GeometricPrimitive::VertexType);
	UINT offset = 0;

	static Drawable* drawables[] = { m_DrawableGrid.get(), m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };
	for (auto const& it : drawables)
	{
		FillBasicEffect(it);

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//======================================== tree billboards =============================================================//

	UINT treeStride = sizeof(TreePointSprite);

	m_BillboardEffect.Bind(m_ImmediateContext.Get());

	m_BillboardEffect.SetViewProj(viewProj);
	m_BillboardEffect.SetSampler(m_ImmediateContext.Get(), m_SamplerAnisotropic.Get());
	m_BillboardEffect.SetTextureArray(m_ImmediateContext.Get(), m_TreeTexArraySRV.Get());

	m_BillboardEffect.Apply(m_ImmediateContext.Get());

	m_ImmediateContext->OMSetBlendState(m_BSAlphaToCoverage.Get(), NULL, 0xffffffff);
	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	m_ImmediateContext->IASetVertexBuffers(0, 1, m_TreePointsVB.GetAddressOf(), &treeStride, &offset);

	m_ImmediateContext->Draw(6, 0);

	m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//======================================== planar shadows ==============================================================//
	
	static Drawable* drawableShadows[] = { m_DrawableTorus.get(),  m_DrawableTeapot.get(), m_DrawableSphere.get() };

	m_SimpleEffect.Bind(m_ImmediateContext.Get());

	m_ImmediateContext->OMSetBlendState(m_BSTransparent.Get(), NULL, 0xffffffff);
	m_ImmediateContext->OMSetDepthStencilState(m_DSSNoDoubleBlend.Get(), 0);

	XMVECTOR shadowPlane = XMPlaneFromPointNormal(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX shadowMatrix = XMMatrixShadow(shadowPlane, -XMLoadFloat3(&m_DirLight.Direction));

	for (auto const& it : drawableShadows)
	{
		XMMATRIX world = it->GetWorld() * shadowMatrix * XMMatrixTranslation(0.0f, 0.001f, 0.0f);
		
		m_SimpleEffect.SetColor(XMVectorSet(0.0f, 0.0f, 0.0f, 0.4f));
		m_SimpleEffect.SetWorldViewProj(world * viewProj);

		m_SimpleEffect.Apply(m_ImmediateContext.Get());
		
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
	static Drawable* transparentDrawables[] = { m_DrawableMirror.get() /*, m_DrawableBox.get()*/ };
	
	m_ImmediateContext->OMSetBlendState(m_BSTransparent.Get(), NULL, 0xffffffff);
	m_ImmediateContext->RSSetState(m_RSCullNone.Get());
	m_ImmediateContext->OMSetDepthStencilState(m_DSSNoDepthWrite.Get(), 0);

	for (auto const& it : transparentDrawables)
	{
		FillBasicEffect(it);

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

	FillBasicEffect(m_DrawableMirror.get());

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
	m_BasicEffect.SetDirectionalLight(Helpers::GetReflectedLight(m_DirLight, reflectionMatrix));
	m_BasicEffect.SetPointLight(Helpers::GetReflectedLight(m_PointLight, reflectionMatrix));
	m_BasicEffect.SetSpotLight(Helpers::GetReflectedLight(m_SpotLight, reflectionMatrix));
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());

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

		m_BasicEffect.SetWorld(reflectedWorld);
		m_BasicEffect.SetWorldViewProj(reflectedWorld * viewProj);
		m_BasicEffect.SetTextureTransform(XMLoadFloat4x4(&it->TextureTransform));
		m_BasicEffect.SetMaterial(it->Material);
		m_BasicEffect.SetTexture(m_ImmediateContext.Get(), it->TextureSRV.Get());

		m_BasicEffect.Apply(m_ImmediateContext.Get());

		m_ImmediateContext->IASetVertexBuffers(0, 1, it->VertexBuffer.GetAddressOf(), &stride, &offset);
		m_ImmediateContext->IASetIndexBuffer(it->IndexBuffer.Get(), it->IndexBufferFormat, 0);
		m_ImmediateContext->DrawIndexed(it->IndexCount, 0, 0);
	}

	//Restore lights to their old positions/directions
	m_BasicEffect.SetDirectionalLight(m_DirLight);
	m_BasicEffect.SetPointLight(m_PointLight);
	m_BasicEffect.SetSpotLight(m_SpotLight);
	m_BasicEffect.ApplyPerFrameConstants(m_ImmediateContext.Get());
}

void DemoScene::FillBasicEffect(Drawable* drawable)
{
	XMMATRIX viewProj = XMLoadFloat4x4(&m_CameraView) * XMLoadFloat4x4(&m_CameraProjection);

	m_BasicEffect.SetWorld(drawable->GetWorld());
	m_BasicEffect.SetWorldViewProj(drawable->GetWorld() * viewProj);
	m_BasicEffect.SetTextureTransform(XMLoadFloat4x4(&drawable->TextureTransform));
	m_BasicEffect.SetMaterial(drawable->Material);
	m_BasicEffect.SetTexture(m_ImmediateContext.Get(), drawable->TextureSRV.Get());

	m_BasicEffect.Apply(m_ImmediateContext.Get());
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

