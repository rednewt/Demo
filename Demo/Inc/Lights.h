#pragma once

#include <DirectXMath.h>

struct Material
{
	Material()
	{
		//By default, the material receives all the diffuse/ambient light and no specular light
		Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 32.0f); 
	}

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = SpecPower
	
};

static_assert(sizeof(Material) % 16 == 0, "Material struct not 16-byte aligned");

struct PointLight
{
	PointLight() :
		Range(100.0f)
	{
		Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attenuation = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);

		Ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; 
	
	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Attenuation;
	float pad;
};

static_assert(sizeof(PointLight) % 16 == 0, "PointLight struct not 16-byte aligned");

struct SpotLight
{
	SpotLight() :
		Range(100.0f),
		SpotPower(32.0f)
	{
		Ambient = DirectX::XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		Position = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		Attenuation = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);

		SetDirection(DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f));
	}

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular;

	DirectX::XMFLOAT3 Direction;
	float SpotPower;

	DirectX::XMFLOAT3 Position;
	float Range;

	DirectX::XMFLOAT3 Attenuation;
	float pad;

	void SetDirection(DirectX::FXMVECTOR direction)
	{
		DirectX::XMVECTOR v = DirectX::XMVector3Normalize(direction);
		DirectX::XMStoreFloat3(&Direction, v);
	}

	void SetDirection(const DirectX::XMFLOAT3& direction)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&direction);
		SetDirection(v);
	}

	void SetPosition(DirectX::FXMVECTOR position)
	{
		DirectX::XMStoreFloat3(&Position, position);
	}
};

static_assert(sizeof(SpotLight) % 16 == 0, "SpotLight struct not 16-byte aligned");


struct DirectionalLight
{
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; 
	DirectX::XMFLOAT3 Direction;
	float pad;

	DirectionalLight()
	{
		SetDirection(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));

		Ambient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	void SetDirection(DirectX::FXMVECTOR direction)
	{
		DirectX::XMVECTOR v = DirectX::XMVector3Normalize(direction);
		DirectX::XMStoreFloat3(&Direction, v);
	}

	void SetDirection(const DirectX::XMFLOAT3& direction)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&direction);
		SetDirection(v);
	}
};

static_assert(sizeof(DirectionalLight) % 16 == 0, "DirectionalLight struct not 16-byte aligned");



struct FogProperties
{
	float FogStart;
	float FogRange;
	float FogEnabled; // Set to <= 0 to disable
	float pad;
	DirectX::XMFLOAT4 FogColor;

	FogProperties() :
		FogStart(20.0f),
		FogRange(100.0f),
		FogEnabled(1.0f)
	{
		DirectX::XMStoreFloat4(&FogColor, DirectX::Colors::Silver);
	}
};

static_assert(sizeof(FogProperties) % 16 == 0, "FogProperties struct not 16-byte aligned");
