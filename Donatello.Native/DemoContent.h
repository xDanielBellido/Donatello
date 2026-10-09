/*
    ===========================================================================
    DemoContent
    ===========================================================================

    Creates and owns the shaders, graphics pipeline and geometry used by
    the demonstration. Provides a render item for drawing that content.
*/

#pragma once

#include "GraphicsPipeline.h"
#include "GpuMesh.h"
#include "RenderItem.h"

#include <d3dcompiler.h>

namespace Donatello
{
    class DemoContent final
    {
    private:
        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader_;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader_;
        GraphicsPipeline graphicsPipeline_;
        GpuMesh mesh_;

        HRESULT CompileShaders();
        HRESULT CreatePipelineState(ID3D12Device* device);
        HRESULT CreateVertexBuffer(ID3D12Device* device);

    public:
        DemoContent() = default;
        ~DemoContent() = default;

        DemoContent(const DemoContent&) = delete;
        DemoContent& operator=(const DemoContent&) = delete;

        HRESULT Initialize(ID3D12Device* device);

        RenderItem GetRenderItem() const { return { graphicsPipeline_, mesh_, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST }; }
    };
}