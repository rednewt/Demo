#pragma once

#include "DemoBase.h"

namespace
{
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 Tex;
	};

	struct SimpleConstantBuffer
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	};
	
}

class DemoScene : public DemoBase
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_SimpleVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_SimplePixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_SimpleVertexLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CubeVB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CubeIB;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_SimpleConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CubeTexSRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerDefault;

	SimpleConstantBuffer m_SimpleCBData;

	D3D11_INPUT_ELEMENT_DESC m_SimpleVertexDesc[2];

public:
	DemoScene(const HWND& hwnd);
	~DemoScene();

	bool Initialize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

private:
	void Clear();
	bool CreateDeviceDependentResources();
	bool CreateBuffers();
};