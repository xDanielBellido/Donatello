#include "pch.h"
#include "SwapChain.h"

namespace Donatello
{
    HRESULT SwapChain::Initialize(IDXGIFactory2* factory, ID3D12CommandQueue* queue, HWND window)
    {
        if (!factory || !queue)
        {
            return E_POINTER;
        }

        if (!IsWindow(window))
        {
            return E_INVALIDARG;
        }

        if (swapChain_)
        {
            return E_UNEXPECTED;
        }

        DXGI_SWAP_CHAIN_DESC1 description{};
        description.Width = 0;
        description.Height = 0;
        description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        description.SampleDesc.Count = 1;
        description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        description.BufferCount = 2;
        description.Scaling = DXGI_SCALING_STRETCH;
        description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        description.Flags = 0;

        Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
        HRESULT result = factory->CreateSwapChainForHwnd(queue, window, &description, nullptr, nullptr, swapChain.GetAddressOf());

        if (FAILED(result))
        {
            return result;
        }

        result = swapChain.As(&swapChain_);

        if (FAILED(result))
        {
            return result;
        }

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: viewport swap chain created.\n");
#endif

        return S_OK;
    }
}