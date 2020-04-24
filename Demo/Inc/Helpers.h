#pragma once

#include <DirectXMath.h>
#include "Lights.h"
#include <vector>
#include "VertexTypes.h"

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
}
  
namespace Helpers
{
	inline DirectX::XMMATRIX ComputeInverseTranspose(DirectX::FXMMATRIX matrix)
	{
		DirectX::XMMATRIX A = matrix;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	inline DirectX::XMFLOAT4X4 ComputeInverseTranspose(const DirectX::XMFLOAT4X4& matrix)
	{
		DirectX::XMMATRIX m = ComputeInverseTranspose(DirectX::XMLoadFloat4x4(&matrix));	
		DirectX::XMFLOAT4X4 f;
		DirectX::XMStoreFloat4x4(&f, m);
		
		return f;
	}

	template<typename T>
	inline void UpdateConstantBuffer(ID3D11DeviceContext* const ctx, ID3D11Buffer* const cbuffer, T* const pData)
	{
		assert(ctx != nullptr);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		DX::ThrowIfFailed(ctx->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
		memcpy(mappedRes.pData, pData, sizeof(T));
		ctx->Unmap(cbuffer, 0);
	}

	template<typename T>
	inline void CreateConstantBuffer(ID3D11Device* const device, T* const pInitialData, ID3D11Buffer** outppBuffer)
	{
		assert(device != nullptr);

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.ByteWidth = sizeof(T);

		D3D11_SUBRESOURCE_DATA cbData = {};
		cbData.pSysMem = pInitialData;
		
		DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, &cbData, outppBuffer));
	}

	template<typename T>
	inline void CreateConstantBuffer(ID3D11Device* const device, ID3D11Buffer** outppBuffer)
	{
		assert(device != nullptr);

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.ByteWidth = sizeof(T);

		DX::ThrowIfFailed(device->CreateBuffer(&cbDesc, nullptr, outppBuffer));
	}

	inline DirectionalLight GetReflectedLight(DirectionalLight& const light, DirectX::FXMMATRIX reflectionMatrix)
	{
		DirectionalLight reflected = light;
		DirectX::XMVECTOR refDirection =  XMVector3TransformNormal(DirectX::XMLoadFloat3(&light.Direction), reflectionMatrix);
		XMStoreFloat3(&reflected.Direction, refDirection);
		
		return reflected;
	}

	inline PointLight GetReflectedLight(PointLight& const light, DirectX::FXMMATRIX reflectionMatrix)
	{
		PointLight reflected = light;
		DirectX::XMVECTOR refPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&light.Position), reflectionMatrix);
		XMStoreFloat3(&reflected.Position, refPosition);

		return reflected;
	}

	inline SpotLight GetReflectedLight(SpotLight& const light, DirectX::FXMMATRIX reflectionMatrix)
	{
		SpotLight reflected = light;
		
		DirectX::XMVECTOR refPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&light.Position), reflectionMatrix);
		DirectX::XMVECTOR refDirection = XMVector3TransformNormal(DirectX::XMLoadFloat3(&light.Direction), reflectionMatrix);
		
		XMStoreFloat3(&reflected.Position, refPosition);
		XMStoreFloat3(&reflected.Direction, refDirection);

		return reflected;
	}


	//tessellation is number of vertices in each dimension
	//so for tessellation of 8 -- 8*8 vertices will be generated
	void CreateGrid(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices,
		const uint16_t width, const uint16_t depth, uint16_t tessellation = 32);
};


struct Drawable
{
	~Drawable() = default;
	Drawable() :
		IndexCount(0), VertexCount(0),
		IndexBufferFormat(DXGI_FORMAT_R16_UINT)
	{
		XMStoreFloat4x4(&ViewProjTransform, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&WorldTransform, DirectX::XMMatrixIdentity());
		XMStoreFloat4x4(&TextureTransform, DirectX::XMMatrixIdentity());
	}

	template<typename VertexType, typename IndexType>
	void Create(ID3D11Device* const device, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices)
	{
		assert(device != nullptr);
		IndexCount = static_cast<uint32_t>(indices.size());
		VertexCount = static_cast<uint32_t>(vertices.size());

		if (sizeof(IndexType) == 4)
			IndexBufferFormat = DXGI_FORMAT_R32_UINT;
		else
			static_assert(sizeof(IndexType) == 2, "Unknown index buffer format");

		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vbDesc.ByteWidth = sizeof(VertexType) * VertexCount;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vbData = {};
		vbData.pSysMem = vertices.data();

		DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData, VertexBuffer.ReleaseAndGetAddressOf()));

		D3D11_BUFFER_DESC ibDesc = {};
		ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
		ibDesc.ByteWidth = sizeof(IndexType) * IndexCount;
		ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA ibData = {};
		ibData.pSysMem = indices.data();

		DX::ThrowIfFailed(device->CreateBuffer(&ibDesc, &ibData, IndexBuffer.ReleaseAndGetAddressOf()));
	}

	DirectX::XMFLOAT4X4 GetWorldViewProj()
	{
		DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&WorldTransform);
		DirectX::XMMATRIX viewProj = DirectX::XMLoadFloat4x4(&ViewProjTransform);

		DirectX::XMFLOAT4X4 worldViewProj;
		DirectX::XMStoreFloat4x4(&worldViewProj, world * viewProj);

		return worldViewProj;
	}

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	uint32_t IndexCount;
	uint32_t VertexCount;
	DXGI_FORMAT IndexBufferFormat;
	DirectX::XMFLOAT4X4 ViewProjTransform;
	DirectX::XMFLOAT4X4 WorldTransform;
	DirectX::XMFLOAT4X4 TextureTransform;
	Material Material;
};

