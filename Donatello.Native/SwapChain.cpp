#include "pch.h"
#include "SwapChain.h"

#include <cwchar>

namespace Donatello
{
    HRESULT SwapChain::Initialize(ID3D12Device* device, IDXGIFactory2* factory, ID3D12CommandQueue* queue, HWND window)
    {
        if (!device || !factory || !queue)
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
        description.BufferCount = BufferCount;
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

        Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;
        result = swapChain.As(&swapChain3);

        if (FAILED(result))
        {
            return result;
        }

        // Reserve one descriptor slot for each back buffer.
        D3D12_DESCRIPTOR_HEAP_DESC heapDescription{};
        heapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        heapDescription.NumDescriptors = BufferCount;
        heapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
        result = device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(rtvHeap.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        const UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BufferCount> backBuffers{};

        // Retrieve each existing image and create its render target view.
        for (UINT index = 0; index < BufferCount; ++index)
        {
            result = swapChain3->GetBuffer(index, IID_PPV_ARGS(backBuffers[index].GetAddressOf()));

            if (FAILED(result))
            {
                return result;
            }

            device->CreateRenderTargetView(backBuffers[index].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += descriptorSize;
        }

        // Keep the objects only after initialisation has succeeded.
        swapChain_ = swapChain3;
        rtvHeap_ = rtvHeap;
        backBuffers_ = backBuffers;
        rtvDescriptorSize_ = descriptorSize;

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: viewport swap chain and render target views created.\n");
#endif

        return S_OK;
    }

    ID3D12Resource* SwapChain::GetCurrentBackBuffer() const
    {
        if (!swapChain_)
        {
            return nullptr;
        }

        const UINT index = swapChain_->GetCurrentBackBufferIndex();
        return backBuffers_[index].Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE SwapChain::GetCurrentRenderTargetView() const
    {
        if (!swapChain_ || !rtvHeap_)
        {
            return {};
        }

        const UINT index = swapChain_->GetCurrentBackBufferIndex();
        D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += static_cast<SIZE_T>(index) * rtvDescriptorSize_;

        return handle;
    }

    HRESULT SwapChain::Present()
    {
        if (!swapChain_)
        {
            return E_UNEXPECTED;
        }

        return swapChain_->Present(1, 0);
    }

    HRESULT SwapChain::ResizeToWindow(ID3D12Device* device)
    {
        if (!device)
        {
            return E_POINTER;
        }

        if (!swapChain_ || !rtvHeap_ || !GetCurrentBackBuffer())
        {
            return E_UNEXPECTED;
        }

        HWND window = nullptr;
        HRESULT result = swapChain_->GetHwnd(&window);

        if (FAILED(result))
        {
            return result;
        }

        RECT clientArea{};

        if (!GetClientRect(window, &clientArea))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        const LONG clientWidth = clientArea.right - clientArea.left;
        const LONG clientHeight = clientArea.bottom - clientArea.top;

        if (clientWidth <= 0 || clientHeight <= 0)
        {
            return S_FALSE;
        }

        const UINT width = static_cast<UINT>(clientWidth);
        const UINT height = static_cast<UINT>(clientHeight);

        DXGI_SWAP_CHAIN_DESC1 description{};
        result = swapChain_->GetDesc1(&description);

        if (FAILED(result))
        {
            return result;
        }

        if (description.Width == width && description.Height == height)
        {
            return S_OK;
        }

        // The caller must have finished previous GPU work before reaching this point.
        for (auto& backBuffer : backBuffers_)
        {
            backBuffer.Reset();
        }

        result = swapChain_->ResizeBuffers(BufferCount, width, height, description.Format, description.Flags);

        if (FAILED(result))
        {
            return result;
        }

        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, BufferCount> backBuffers{};
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();

        for (UINT index = 0; index < BufferCount; ++index)
        {
            result = swapChain_->GetBuffer(index, IID_PPV_ARGS(backBuffers[index].GetAddressOf()));

            if (FAILED(result))
            {
                return result;
            }

            device->CreateRenderTargetView(backBuffers[index].Get(), nullptr, rtvHandle);
            rtvHandle.ptr += rtvDescriptorSize_;
        }

        backBuffers_ = backBuffers;

#if defined(_DEBUG)
        wchar_t message[128]{};
        swprintf_s(message, L"Donatello: viewport buffers resized to %u x %u pixels.\n", width, height);
        OutputDebugStringW(message);
#endif

        return S_OK;
    }

}