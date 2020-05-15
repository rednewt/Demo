#pragma once

#include <d3d11.h>
#include <wrl\client.h>
#include "Helpers.h"

class InputLayout
{
private:
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_VertexLayout;

public:
	InputLayout() = default;
	InputLayout(InputLayout const&) = delete;
	InputLayout& operator=(InputLayout const&) = delete;

	void Create(ID3D11Device* device, const D3D11_INPUT_ELEMENT_DESC* pDesc, UINT numElements,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength)
	{
		DX::ThrowIfFailed(device->CreateInputLayout(pDesc, numElements, pVertexShaderByteCode, vertexShaderByteLength, m_VertexLayout.ReleaseAndGetAddressOf()));
	}

	void Bind(ID3D11DeviceContext* context) const
	{
		context->IASetInputLayout(m_VertexLayout.Get());
	}
};

class Shader
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	std::shared_ptr<InputLayout> m_InputLayout;
public:
	Shader() = default;
	Shader(Shader const&) = delete;
	Shader& operator=(Shader const&) = delete;


	void Create(ID3D11Device* device, const std::shared_ptr<InputLayout>& inputLayout,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T pixelShaderByteLength) 
	{
		assert(device != nullptr);

		m_InputLayout = inputLayout;
		DX::ThrowIfFailed(device->CreateVertexShader(pVertexShaderByteCode, vertexShaderByteLength, nullptr, m_VertexShader.ReleaseAndGetAddressOf()));
		DX::ThrowIfFailed(device->CreatePixelShader(pPixelShaderByteCode, pixelShaderByteLength, nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
	}

	void Bind(ID3D11DeviceContext* context) const
	{
		assert(context != nullptr);

		m_InputLayout->Bind(context);

		context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	}
};