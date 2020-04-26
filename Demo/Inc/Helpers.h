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

	inline DirectX::XMFLOAT4X4 XMMatrixToStorage(DirectX::FXMMATRIX m)
	{
		DirectX::XMFLOAT4X4 f;
		XMStoreFloat4x4(&f, m);

		return f;
	}

	template<typename T>
	inline void UpdateConstantBuffer(ID3D11DeviceContext* const context, ID3D11Buffer* const cbuffer, T* const pData)
	{
		assert(context != nullptr);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		DX::ThrowIfFailed(context->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
		memcpy(mappedRes.pData, pData, sizeof(T));
		context->Unmap(cbuffer, 0);
	}

	template<typename T>
	inline void CreateBuffer(ID3D11Device* const device, std::vector<T> data, D3D11_BIND_FLAG bindFlag, ID3D11Buffer** outppBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.ByteWidth = sizeof(T) * data.size();
		desc.BindFlags = bindFlag;
		
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data.data();

		DX::ThrowIfFailed(device->CreateBuffer(&desc, &initData, outppBuffer));
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

	inline DirectionalLight GetReflectedLight(const DirectionalLight& light, DirectX::FXMMATRIX reflectionMatrix)
	{
		DirectionalLight reflected = light;
		DirectX::XMVECTOR refDirection =  XMVector3TransformNormal(DirectX::XMLoadFloat3(&light.Direction), reflectionMatrix);
		XMStoreFloat3(&reflected.Direction, refDirection);
		
		return reflected;
	}

	inline PointLight GetReflectedLight(const PointLight& light, DirectX::FXMMATRIX reflectionMatrix)
	{
		PointLight reflected = light;
		DirectX::XMVECTOR refPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&light.Position), reflectionMatrix);
		XMStoreFloat3(&reflected.Position, refPosition);

		return reflected;
	}

	inline SpotLight GetReflectedLight(const SpotLight& light, DirectX::FXMMATRIX reflectionMatrix)
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



