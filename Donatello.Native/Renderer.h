/*
    ===========================================================================
    Renderer
    ===========================================================================

    Owns the graphics device, command queue, swap chain and command context.

    Coordinates initialisation, viewport resizing, command recording,
    GPU submission, completion waits and presentation.

    Currently renders a solid colour using a synchronous rendering path.
*/

#pragma once

#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "CommandContext.h"

#include "RenderItem.h"

#include <span>

namespace Donatello
{
    class Renderer final
    {
    private:
        GraphicsDevice device_;
        CommandQueue commandQueue_;
        SwapChain swapChain_;
        CommandContext commandContext_;

    public:
        Renderer() = default;
        ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        HRESULT             Initialize();
        HRESULT             CreateViewport(HWND window);
        HRESULT             RenderFrame(std::span<const RenderItem> items);
        HRESULT WaitForIdle();

        ID3D12Device*       GetDevice() const { return device_.GetDevice(); };
        HRESULT             GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const;
    };
}