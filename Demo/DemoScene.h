#pragma once

#include "DemoBase.h"

class DemoScene : public DemoBase
{
public:
	DemoScene(const HWND& hwnd, UINT ClientWidth, UINT ClientHeight);
	~DemoScene();

	void UpdateScene(float dt) override;
	void DrawScene() override;
};