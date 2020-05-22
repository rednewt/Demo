#pragma once

#include "DemoBase.h"
#include "ConstantBuffer.h"
#include "Shaders.h"

struct Drawable;

class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;



	struct GS_PS_CbConstants_SimpleShader
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4 Color;

	} m_CbConstantsData;

	static_assert(sizeof(GS_PS_CbConstants_SimpleShader) % 16 == 0, "struct not 16-byte aligned");



	std::unique_ptr<Drawable> m_DrawableBox;
	std::unique_ptr<Drawable> m_DrawableSphere;
	std::unique_ptr<Drawable> m_DrawableTorus;
	std::unique_ptr<Drawable> m_DrawableTeapot;
	std::unique_ptr<Drawable> m_DrawableGrid;
	std::unique_ptr<Drawable> m_DrawableMirror;

	BasicEffect m_BasicEffect;
	Shader m_SimpleShader;

	//basic shader constant buffers

	//simple shader constant buffers
	ConstantBuffer<GS_PS_CbConstants_SimpleShader> m_CbConstants;

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

	DirectionalLight m_DirLight;
	PointLight m_PointLight;
	SpotLight m_SpotLight;
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
	void SetBasicEffectParams(Drawable* const drawable);
};