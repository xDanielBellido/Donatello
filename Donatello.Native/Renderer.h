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

#include "GraphicsPipeline.h"
#include "GpuMesh.h"

#include <d3dcompiler.h>

namespace Donatello
{
    class Renderer final
    {
    private:
        GraphicsDevice device_;
        CommandQueue commandQueue_;
        SwapChain swapChain_;
        CommandContext commandContext_;

        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader_;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader_;

        GraphicsPipeline graphicsPipeline_;

        GpuMesh mesh_;

        HRESULT CompileShaders();
        HRESULT CreatePipelineState();
        HRESULT CreateVertexBuffer();

    public:
        Renderer() = default;
        ~Renderer() = default;

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;

        HRESULT             Initialize();
        HRESULT             CreateViewport(HWND window);
        HRESULT             RenderFrame();
        HRESULT             GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const;
    };
}