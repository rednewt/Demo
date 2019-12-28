#pragma once


#include <Windows.h>
#include <string>
#include <assert.h>
#include <memory>
#include <DirectXMath.h>
#include <DirectXColors.h>

#pragma comment(lib, "d3d11.lib")

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