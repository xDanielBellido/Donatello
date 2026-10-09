/*
    ===========================================================================
    GraphicsPipeline
    ===========================================================================

    Owns a graphics pipeline state and its compatible root signature.
    Creates both objects from descriptions supplied by the caller.
*/

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace Donatello
{
    class GraphicsPipeline final
    {
    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    public:
        GraphicsPipeline() = default;
        ~GraphicsPipeline() = default;

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        HRESULT Initialize(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& rootDescription, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineDescription);

        ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
        ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }
    };
}