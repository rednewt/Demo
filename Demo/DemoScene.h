#pragma once

#include "DemoBase.h"

namespace VertexStructure
{
	struct SimpleVertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT4 Color;
	};
}

class DemoScene : public DemoBase
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_SimpleVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_SimplePixelShader;
	
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_SimpleVertexLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_SimpleVertexBuffer;

	D3D11_INPUT_ELEMENT_DESC m_SimpleVertexDesc[2];

public:
	DemoScene(const HWND& hwnd);
	~DemoScene();

	bool Initialize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;
};