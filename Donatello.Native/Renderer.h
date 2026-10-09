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

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

        HRESULT CompileShaders();
        HRESULT CreateRootSignature();
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