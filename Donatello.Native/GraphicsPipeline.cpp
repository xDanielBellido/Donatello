#include "pch.h"
#include "GraphicsPipeline.h"

namespace Donatello
{
    HRESULT GraphicsPipeline::Initialize(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& rootDescription, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& pipelineDescription)
    {
        if (!device)
        {
            return E_POINTER;
        }

        if (pipelineState_)
        {
            return E_UNEXPECTED;
        }

        Microsoft::WRL::ComPtr<ID3DBlob> serialised;
        Microsoft::WRL::ComPtr<ID3DBlob> diagnostics;

        HRESULT result = D3D12SerializeRootSignature(&rootDescription, D3D_ROOT_SIGNATURE_VERSION_1, serialised.GetAddressOf(), diagnostics.GetAddressOf());

        if (diagnostics)
        {
            OutputDebugStringA(static_cast<const char*>(diagnostics->GetBufferPointer()));
        }

        if (FAILED(result))
        {
            return result;
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

        result = device->CreateRootSignature(0, serialised->GetBufferPointer(), serialised->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC description = pipelineDescription;
        description.pRootSignature = rootSignature.Get();

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

        result = device->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(pipelineState.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        rootSignature_ = rootSignature;
        pipelineState_ = pipelineState;

        return S_OK;
    }
}