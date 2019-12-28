#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#define MINIMUM_CLIENT_WIDTH 800
#define MINIMUM_CLIENT_HEIGHT 600

class DemoBase
{
protected:
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_ImmediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_DepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_DepthStencilView;

	D3D11_VIEWPORT m_ScreenViewport;
	
	HWND m_MainWindow;

	UINT m_ClientWidth;
	UINT m_ClientHeight;
	UINT m_MSAAQuality;
public:
	DemoBase(const HWND& hwnd, UINT ClientWidth, UINT ClientHeight);
	virtual ~DemoBase();

	virtual bool Initialize();
	void OnResize();

	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;

};