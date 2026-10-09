#include "pch.h"
#include "Renderer.h"

namespace Donatello
{
    
    HRESULT Renderer::Initialize()
    {
        HRESULT result = device_.Initialize();

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.Initialize(device_.GetDevice());

        if (FAILED(result))
        {
            return result;
        }

        result = commandContext_.Initialize(device_.GetDevice());

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: initial GPU fence wait completed.\n");
#endif

        return S_OK;
    }

    HRESULT Renderer::CreateViewport(HWND window)
    {
        return swapChain_.Initialize(device_.GetDevice(), device_.GetFactory(), commandQueue_.GetQueue(), window);
    }

    HRESULT Renderer::GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const
    {
        return device_.GetAdapterDescription(description);
    }

    HRESULT Renderer::RenderFrame(std::span<const RenderItem> items)
    {
        // Finish previous work before resizing or reusing command memory.
        HRESULT result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

        result = swapChain_.ResizeToWindow(device_.GetDevice());

        if (result == S_FALSE)
        {
            return S_FALSE;
        }

        if (FAILED(result))
        {
            return result;
        }

        ID3D12Resource* backBuffer = swapChain_.GetCurrentBackBuffer();

        if (!backBuffer)
        {
            return E_UNEXPECTED;
        }

        const float colour[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain_.GetCurrentRenderTargetView();

        result = commandContext_.BeginFrame(backBuffer, rtv, colour);

        if (FAILED(result))
        {
            return result;
        }

        for (const RenderItem& item : items)
        {
            result = commandContext_.Draw(item.pipeline, item.mesh, item.topology);

            if (FAILED(result))
            {
                commandContext_.EndFrame(backBuffer);
                return result;
            }
        }

        result = commandContext_.EndFrame(backBuffer);

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.Execute(commandContext_.GetCommandList());

        if (FAILED(result))
        {
            return result;
        }

        // Keep this first rendering path synchronous.
        result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

        return swapChain_.Present();
    }

    HRESULT Renderer::WaitForIdle()
    {
        if (!commandQueue_.GetQueue())
        {
            return S_OK;
        }

        return commandQueue_.WaitForIdle();
    }

}