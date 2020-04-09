#pragma once

#include <DirectXMath.h>


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
	static DirectX::XMMATRIX ComputeInverseTranspose(DirectX::FXMMATRIX matrix)
	{
		DirectX::XMMATRIX A = matrix;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	static DirectX::XMFLOAT4X4 ComputeInverseTranspose(const DirectX::XMFLOAT4X4& matrix)
	{
		DirectX::XMMATRIX m = ComputeInverseTranspose(DirectX::XMLoadFloat4x4(&matrix));	
		DirectX::XMFLOAT4X4 f;
		DirectX::XMStoreFloat4x4(&f, m);
		
		return f;
	}

	template<typename T>
	static void UpdateConstantBuffer(ID3D11DeviceContext* const ctx, ID3D11Buffer* const cbuffer, T* const newData)
	{
		assert(ctx != nullptr);

		D3D11_MAPPED_SUBRESOURCE mappedRes;
		DX::ThrowIfFailed(ctx->Map(cbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedRes));
		memcpy(mappedRes.pData, newData, sizeof(T));
		ctx->Unmap(cbuffer, 0);
	}
};