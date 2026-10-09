/*
    ===========================================================================
    SwapChain
    ===========================================================================

    Owns the DXGI swap chain associated with a native viewport window.

    Holds references to its two back buffers and stores their render target
    views in a descriptor heap, allowing them to be used as drawing targets.

    Provides access to the current back buffer and presents rendered images.
    Does not yet handle viewport resizing.
*/

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <array>

namespace Donatello
{
    class SwapChain final
    {
    private:
    private:
        static constexpr UINT BufferCount = 2;

        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain_;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BufferCount> backBuffers_;
        UINT rtvDescriptorSize_ = 0;

    public:
        SwapChain() = default;
        ~SwapChain() = default;

        SwapChain(const SwapChain&) = delete;
        SwapChain& operator=(const SwapChain&) = delete;

        HRESULT Initialize(ID3D12Device* device, IDXGIFactory2* factory, ID3D12CommandQueue* queue, HWND window);

        ID3D12Resource* GetCurrentBackBuffer() const;
        D3D12_CPU_DESCRIPTOR_HANDLE  GetCurrentRenderTargetView() const;
        HRESULT                     Present();
    };
}