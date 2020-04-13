#pragma once

#include <d3d11.h>
#include <wrl/client.h>

//use constructor for initializing variables
//use Initialize() for creating resources

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
	UINT M_MSAASampleCount;

	DXGI_FORMAT m_BackBufferFormat;
	DXGI_FORMAT m_DepthStencilBufferFormat;
	DXGI_FORMAT m_DepthStencilViewFormat;
public:
	DemoBase(const HWND& hwnd);
	virtual ~DemoBase();

	virtual bool Initialize();
	virtual void OnResize();

	virtual void UpdateScene(float dt) = 0;
	virtual void DrawScene() = 0;

private:
	void Init_ImGui();
protected:
	void ImGui_NewFrame();
};