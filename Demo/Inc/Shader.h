#pragma once

#include <d3d11.h>
#include <wrl\client.h>
#include "Helpers.h"

class Shader
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_VertexLayout;
public:
	Shader() = default;
	Shader(Shader const&) = delete;
	Shader& operator=(Shader const&) = delete;


	void Create(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* pDesc, UINT NumElements, 
		const void* pVertexShaderByteCode, SIZE_T VertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T PixelShaderByteLength) 
	{
		assert(device != nullptr);

		DX::ThrowIfFailed(device->CreateInputLayout(pDesc, NumElements, pVertexShaderByteCode, VertexShaderByteLength, m_VertexLayout.ReleaseAndGetAddressOf()));

		DX::ThrowIfFailed(device->CreateVertexShader(pVertexShaderByteCode, VertexShaderByteLength, nullptr, m_VertexShader.ReleaseAndGetAddressOf()));
		
		DX::ThrowIfFailed(device->CreatePixelShader(pPixelShaderByteCode, PixelShaderByteLength, nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
	}

	void Bind(ID3D11DeviceContext* context)
	{
		assert(context != nullptr);

		context->IASetInputLayout(m_VertexLayout.Get());
		context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	}
};