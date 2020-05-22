#include "pch.h"
#include "Shaders.h"

namespace
{
	#include "Shaders\Compiled\SimplePS.h"
	#include "Shaders\Compiled\SimpleVS.h"
}

using namespace DirectX;

void SimpleEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	Shader::Create(device, inputLayout, g_SimpleVS, sizeof(g_SimpleVS), g_SimplePS, sizeof(g_SimplePS));
	m_CbConstants.Create(device);
}

void SimpleEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbConstantsData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void SimpleEffect::SetColor(DirectX::FXMVECTOR color)
{
	XMStoreFloat4(&m_CbConstantsData.Color, color);
}

void SimpleEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbConstants.SetData(context, m_CbConstantsData);
}

void SimpleEffect::Bind(ID3D11DeviceContext* context) const
{
	Shader::Bind(context);
	context->VSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_CbConstants.GetAddressOf());
}
