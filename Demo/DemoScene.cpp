#include "pch.h"
#include "DemoScene.h"

DemoScene::DemoScene(const HWND& hwnd, UINT ClientWidth, UINT ClientHeight) :
	DemoBase(hwnd, ClientWidth, ClientHeight)
{
	
}

DemoScene::~DemoScene()
{
}


void DemoScene::UpdateScene(float dt)
{
}

void DemoScene::DrawScene()
{
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), DirectX::Colors::Black);
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);


	m_SwapChain->Present(0, 0);
}
