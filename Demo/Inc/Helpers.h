#pragma once

#include <DirectXMath.h>
#include "Lights.h"
#include <vector>
#include <VertexTypes.h>

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


	/* tessellation is number of vertices in each dimension
	/  so for tessellation of 8, 8*8 vertices will be generated */
	void CreateGrid(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices,
		const uint16_t width, const uint16_t depth, uint16_t tessellation = 32);
};



