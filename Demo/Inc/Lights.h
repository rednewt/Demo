#pragma once

#include <DirectXMath.h>

struct Material
{
	Material()
	{
		//By default, the material receives all the light
		Ambient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 16.0f);
	}

	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; // w = SpecPower
};

struct DirectionalLight
{
	DirectX::XMFLOAT4 Ambient;
	DirectX::XMFLOAT4 Diffuse;
	DirectX::XMFLOAT4 Specular; //w isn't of much use here
	DirectX::XMFLOAT3 Direction;
	float pad;

	DirectionalLight()
	{
		SetDirection(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));

		Ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		Diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);

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