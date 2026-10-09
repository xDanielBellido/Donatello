#include "pch.h"
#include "CommandContext.h"

#include "GraphicsPipeline.h"
#include "GpuMesh.h"

namespace Donatello
{
    HRESULT CommandContext::Initialize(ID3D12Device* device)
    {
        if (!device)
        {
            return E_POINTER;
        }

        if (commandList_)
        {
            return S_OK;
        }

        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
        HRESULT result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(allocator.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
        result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        // Start closed so that recording can begin explicitly later.
        result = commandList->Close();

        if (FAILED(result))
        {
            return result;
        }

        allocator_ = allocator;
        commandList_ = commandList;

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: graphics command context created.\n");
#endif

        return S_OK;
    }

    HRESULT CommandContext::BeginFrame(ID3D12Resource* backBuffer, D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float* colour)
    {
        if (!backBuffer || !colour || rtv.ptr == 0)
        {
            return E_POINTER;
        }

        if (!allocator_ || !commandList_ || recording_)
        {
            return E_UNEXPECTED;
        }

        // The caller must ensure previous GPU work has finished.
        HRESULT result = allocator_->Reset();

        if (FAILED(result))
        {
            return result;
        }

        result = commandList_->Reset(allocator_.Get(), nullptr);

        if (FAILED(result))
        {
            return result;
        }

        // Prepare the image for use as a render target.
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = backBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList_->ResourceBarrier(1, &barrier);

        // Fill the entire image with the requested RGBA colour.
        commandList_->ClearRenderTargetView(rtv, colour, 0, nullptr);

        // Use the current back buffer dimensions, including after resizing.
        const D3D12_RESOURCE_DESC bufferDescription = backBuffer->GetDesc();

        D3D12_VIEWPORT viewport{};
        viewport.Width = static_cast<float>(bufferDescription.Width);
        viewport.Height = static_cast<float>(bufferDescription.Height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        D3D12_RECT scissor{};
        scissor.right = static_cast<LONG>(bufferDescription.Width);
        scissor.bottom = static_cast<LONG>(bufferDescription.Height);

        commandList_->RSSetViewports(1, &viewport);
        commandList_->RSSetScissorRects(1, &scissor);

        // Select the destination for the pixel shader's output.
        commandList_->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

        recording_ = true;

        return S_OK;
    }

    HRESULT CommandContext::Draw(const GraphicsPipeline& pipeline, const GpuMesh& mesh, D3D_PRIMITIVE_TOPOLOGY topology)
    {
        if (!recording_)
        {
            return E_UNEXPECTED;
        }

        ID3D12PipelineState* pipelineState = pipeline.GetPipelineState();
        ID3D12RootSignature* rootSignature = pipeline.GetRootSignature();
        const auto& vertexBufferView = mesh.GetVertexBufferView();

        if (!pipelineState || !rootSignature || vertexBufferView.BufferLocation == 0 || mesh.GetVertexCount() == 0)
        {
            return E_INVALIDARG;
        }

        if (topology == D3D_PRIMITIVE_TOPOLOGY_UNDEFINED)
        {
            return E_INVALIDARG;
        }

        commandList_->SetPipelineState(pipelineState);
        commandList_->SetGraphicsRootSignature(rootSignature);
        commandList_->IASetPrimitiveTopology(topology);
        commandList_->IASetVertexBuffers(0, 1, &vertexBufferView);
        commandList_->DrawInstanced(mesh.GetVertexCount(), 1, 0, 0);

        return S_OK;
    }

    HRESULT CommandContext::EndFrame(ID3D12Resource* backBuffer)
    {
        if (!backBuffer)
        {
            return E_POINTER;
        }

        if (!recording_)
        {
            return E_UNEXPECTED;
        }

        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = backBuffer;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        commandList_->ResourceBarrier(1, &barrier);

        HRESULT result = commandList_->Close();
        recording_ = false;

        return result;
    }

}