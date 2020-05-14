#pragma once

#include "DemoBase.h"
#include "ConstantBuffer.h"
#include "Shader.h"

struct Drawable;
class SimpleDrawable;

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;
	struct VS_PS_ConstantBufferPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInvTranspose;
		DirectX::XMFLOAT4X4 TextureTransform;
		Material Material;
	};
	struct PS_ConstantBufferPerFrame
	{
		DirectionalLight DirLight;
		PointLight PointLight;
		SpotLight SpotLight;

		DirectX::XMFLOAT3 EyePos;
		float pad;

		FogProperties Fog;
	};
	static_assert(sizeof(PS_ConstantBufferPerFrame) % 16 == 0, "PS_ConstantBufferPerFrame struct not 16-byte aligned");
	static_assert(sizeof(VS_PS_ConstantBufferPerObject) % 16 == 0, "VS_PS_ConstantBufferPerObject struct not 16-byte aligned");

	Shader m_BasicEffect;

	std::unique_ptr<Drawable> m_DrawableBox;
	std::unique_ptr<Drawable> m_DrawableSphere;
	std::unique_ptr<Drawable> m_DrawableTorus;
	std::unique_ptr<Drawable> m_DrawableTeapot;
	std::unique_ptr<Drawable> m_DrawableGrid;
	std::unique_ptr<Drawable> m_DrawableMirror;

	ConstantBuffer<VS_PS_ConstantBufferPerObject> m_CbPerObject;
	ConstantBuffer<PS_ConstantBufferPerFrame> m_CbPerFrame;
	VS_PS_ConstantBufferPerObject m_CbPerObjectData;
	PS_ConstantBufferPerFrame m_CbPerFrameData;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerAnisotropic;

	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSTransparent;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSNoColorWrite;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSCullNone;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSWireframe;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSFrontCounterCW;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSNoDepthWrite;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSMarkPixels;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSDrawMarkedOnly;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSNoDoubleBlend;

	DirectX::XMFLOAT4X4 m_CameraView;
	DirectX::XMFLOAT4X4 m_CameraProjection;
public:
	explicit DemoScene(const HWND& hwnd);
	DemoScene(const DemoScene&) = delete;
	~DemoScene() = default;

	bool Initialize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;
private:
	void Clear();
	void Present();
	bool CreateDeviceDependentResources();
	void CreateBuffers();
	void PrepareForRendering();
	void RenderToStencil();
	void RenderReflections();
	void ResetStates();
	void FillPerObjectConstantBuffer(Drawable* const drawable);
};