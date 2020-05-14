#pragma once

//Helper struct quickly put together to make code a bit cleaner while drawing
struct Drawable
{
	~Drawable() = default;
	Drawable(const Drawable&) = delete;

	Drawable() :
		IndexCount(0), VertexCount(0),
		IndexBufferFormat(DXGI_FORMAT_R16_UINT)
	{
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

		Helpers::CreateBuffer(device, vertices, D3D11_BIND_VERTEX_BUFFER, VertexBuffer.ReleaseAndGetAddressOf());
		Helpers::CreateBuffer(device, indices, D3D11_BIND_INDEX_BUFFER, IndexBuffer.ReleaseAndGetAddressOf());
	}

	DirectX::XMMATRIX GetWorld() const { return XMLoadFloat4x4(&WorldTransform); }

	Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV;
	uint32_t IndexCount;
	uint32_t VertexCount;
	DXGI_FORMAT IndexBufferFormat;
	DirectX::XMFLOAT4X4 WorldTransform;
	DirectX::XMFLOAT4X4 TextureTransform;
	Material Material;
};

/*
class SimpleDrawable
{
public:
	enum class Shape
	{
		Cube,
		Sphere
	};

	~SimpleDrawable() = default;
	SimpleDrawable(const SimpleDrawable&) = delete;
	SimpleDrawable& operator=(const SimpleDrawable&) = delete;

	static std::unique_ptr<SimpleDrawable> Create(ID3D11Device* device, Shape shape);
	static std::unique_ptr<SimpleDrawable> Create(ID3D11Device* device);

	void BindShader(ID3D11DeviceContext* context, ID3D11InputLayout* posNormalTexture);
	void UpdateConstantBuffer(ID3D11DeviceContext* context, DirectX::FXMMATRIX worldViewProj, DirectX::FXMVECTOR objectColor = DirectX::Colors::White.v);
	void Draw(ID3D11DeviceContext* context);
private:
	SimpleDrawable();
	void CreateDeviceDependentResources(ID3D11Device* const device);

	struct VS_PS_CbConstants
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4 Color;
	};

	static_assert(sizeof(VS_PS_CbConstants) % 16 == 0, "VS_PS_CbConstants not 16-byte aligned");

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_CbConstants;

	static Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	static Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	static Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;

	VS_PS_CbConstants m_CbConstantsData;
	uint16_t m_IndexCount;
};

*/