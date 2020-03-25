#pragma once

#include "DemoBase.h"
#include "GeometricPrimitive.h"
#include "VertexTypes.h"
namespace
{
	struct SimpleConstantBuffer
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	};

	struct Drawable
	{
		Drawable() :
			IndexCount(0), VertexCount(0),
			IndexBufferFormat(DXGI_FORMAT_R16_UINT)
		{
		}
		~Drawable() = default;

		template<typename Vertex, typename IndexType>
		void Create(ID3D11Device* const device, const std::vector<Vertex>& vertices, const std::vector<IndexType>& indices)
		{
			assert(device != nullptr);
			IndexCount = indices.size();
			VertexCount = vertices.size();

			if (sizeof(IndexType) == 4)
				IndexBufferFormat = DXGI_FORMAT_R32_UINT;
			else
				static_assert(sizeof(IndexType) == 2, "Unknown index buffer format");

			D3D11_BUFFER_DESC vbDesc = {};
			vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
			vbDesc.ByteWidth = sizeof(Vertex) * vertices.size();
			vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

			D3D11_SUBRESOURCE_DATA vbData = {};
			vbData.pSysMem = &vertices[0];

			DX::ThrowIfFailed(device->CreateBuffer(&vbDesc, &vbData, VertexBuffer.ReleaseAndGetAddressOf()));

			D3D11_BUFFER_DESC ibDesc = {};
			ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
			ibDesc.ByteWidth = sizeof(IndexType) * indices.size();
			ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

			D3D11_SUBRESOURCE_DATA ibData = {};
			ibData.pSysMem = &indices[0];

			DX::ThrowIfFailed(device->CreateBuffer(&ibDesc, &ibData, IndexBuffer.ReleaseAndGetAddressOf()));
		}

		Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
		uint32_t IndexCount;
		DXGI_FORMAT IndexBufferFormat;
		uint32_t VertexCount;
	};

}


class DemoScene : public DemoBase
{
private:
	using Super = DemoBase;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_SimpleVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_SimplePixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_SimpleVertexLayout;

	std::unique_ptr<Drawable> m_Cube;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_SimpleConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_CubeTexSRV;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerDefault;

	SimpleConstantBuffer m_SimpleCBData;
public:
	explicit DemoScene(const HWND& hwnd);
	~DemoScene() = default;

	bool Initialize() override;
	void UpdateScene(float dt) override;
	void DrawScene() override;

private:
	DemoScene(const DemoScene&) = default;

	void Clear();
	bool CreateDeviceDependentResources();
	bool CreateBuffers();

};