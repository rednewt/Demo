#pragma once

#include <d3d11.h>
#include <wrl\client.h>
#include "Helpers.h"
#include "ConstantBuffer.h"

class Shader
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
public:
	Shader() = default;
	Shader(Shader const&) = delete;
	Shader& operator=(Shader const&) = delete;

public:
	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T pixelShaderByteLength) 
	{
		assert(device != nullptr);

		m_InputLayout = inputLayout;
		DX::ThrowIfFailed(device->CreateVertexShader(pVertexShaderByteCode, vertexShaderByteLength, nullptr, m_VertexShader.ReleaseAndGetAddressOf()));
		DX::ThrowIfFailed(device->CreatePixelShader(pPixelShaderByteCode, pixelShaderByteLength, nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
	}


	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T pixelShaderByteLength,
		const void* pGeometryShaderByteCode, SIZE_T geometryShaderByteLength)
	{
		Create(device, inputLayout, pVertexShaderByteCode, vertexShaderByteLength, pPixelShaderByteCode, pixelShaderByteLength);

		DX::ThrowIfFailed(device->CreateGeometryShader(pGeometryShaderByteCode, geometryShaderByteLength, nullptr, m_GeometryShader.ReleaseAndGetAddressOf()));
	}

	void Bind(ID3D11DeviceContext* context) const
	{
		assert(context != nullptr);

		context->IASetInputLayout(m_InputLayout.Get());
		context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		context->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
	}
};

class SimpleEffect : Shader
{
private:
	struct VS_PS_CbConstants
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4 Color;

	} m_CbConstantsData;

	static_assert(sizeof(VS_PS_CbConstants) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_PS_CbConstants> m_CbConstants;
public:
	SimpleEffect() = default;
	SimpleEffect(const SimpleEffect&) = delete;
	SimpleEffect& operator=(const SimpleEffect&) = delete;

	SimpleEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetColor(DirectX::FXMVECTOR color);
	void Apply(ID3D11DeviceContext* context);

	void Bind(ID3D11DeviceContext* context) const;
};

class BasicEffect : Shader
{
private:
	struct VS_PS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInvTranspose;
		DirectX::XMFLOAT4X4 TextureTransform;
		Material Material;

	} m_CbPerObjectData;

	struct PS_CbPerFrame
	{
		DirectionalLight DirLight;
		PointLight PointLight;
		SpotLight SpotLight;

		DirectX::XMFLOAT3 EyePos;
		float pad;

		FogProperties Fog;

	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(VS_PS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_PS_CbPerObject> m_CbPerObject;
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	BasicEffect() = default;
	BasicEffect(const BasicEffect&) = delete;
	BasicEffect& operator=(const BasicEffect&) = delete;

	BasicEffect(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout) 
	{ 
		Create(device, inputLayout); 
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorld(DirectX::FXMMATRIX world);
	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetTextureTransform(DirectX::FXMMATRIX texTransform);
	void SetMaterial(const Material& mat);
	void Apply(ID3D11DeviceContext* context);

	void SetDirectionalLight(const DirectionalLight& light);
	void SetPointLight(const PointLight& light);
	void SetSpotLight(const SpotLight& light);
	void SetEyePosition(DirectX::FXMVECTOR eyePos);
	void SetFog(const FogProperties& fog);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);
	
	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);

	void Bind(ID3D11DeviceContext* context) const;
};


class BillboardEffect : Shader
{
private:
	struct GS_CbPerFrame
	{
		DirectX::XMFLOAT3 EyePos;
		float pad;

	} m_CbPerFrameData;

	struct GS_CbPerObject
	{
		DirectX::XMFLOAT4X4 ViewProj;

	} m_CbPerObjectData;

	ConstantBuffer<GS_CbPerFrame> m_CbPerFrame;
	ConstantBuffer<GS_CbPerObject> m_CbPerObject;

	static_assert(sizeof(GS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(GS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");
public:
	BillboardEffect() = default;
	BillboardEffect(const BillboardEffect&) = delete;
	BillboardEffect& operator=(const BillboardEffect&) = delete;

	BillboardEffect(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetEyePos(DirectX::FXMVECTOR eyePos);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);

	void SetViewProj(DirectX::FXMMATRIX viewProj);
	void Apply(ID3D11DeviceContext* context);

	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetTextureArray(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);

	void Bind(ID3D11DeviceContext* context) const;

};