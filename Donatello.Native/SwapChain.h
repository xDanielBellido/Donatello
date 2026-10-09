/*
    ===========================================================================
    SwapChain
    ===========================================================================

    Owns the DXGI swap chain associated with a native viewport window.

    Creates two back buffers that will hold the images rendered for display.
    ComPtr releases the swap chain when this object is destroyed.

    Does not yet present images or handle viewport resizing.
*/

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Donatello
{
    class SwapChain final
    {
    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;

    public:
        SwapChain() = default;
        ~SwapChain() = default;

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        HRESULT             Initialize(IDXGIFactory2* factory, ID3D12CommandQueue* queue, HWND window);
    };
}