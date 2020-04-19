#pragma once

#include "DemoBase.h"

namespace
{
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

}

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_SimpleVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_SimplePixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_SimpleVertexLayout;

	std::unique_ptr<Drawable> m_DrawableBox;
	std::unique_ptr<Drawable> m_DrawableSphere;
	std::unique_ptr<Drawable> m_DrawableTorus;
	std::unique_ptr<Drawable> m_DrawableTeapot;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CbPerObject;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CbPerFrame;
	VS_PS_ConstantBufferPerObject m_CbPerObjectData;
	PS_ConstantBufferPerFrame m_CbPerFrameData;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerAnisotropic;
	Microsoft::WRL::ComPtr<ID3D11BlendState> m_BSTransparent;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_RSCullNone;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_DSSDisableWrite;
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
};