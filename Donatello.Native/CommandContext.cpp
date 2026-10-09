#include "pch.h"
#include "CommandContext.h"

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

    HRESULT CommandContext::RecordClear(ID3D12Resource* backBuffer, D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float* colour)
    {
        if (!backBuffer || !colour || rtv.ptr == 0)
        {
            return E_POINTER;
        }

        if (!allocator_ || !commandList_)
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

        // Prepare the image for presentation.
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        commandList_->ResourceBarrier(1, &barrier);

        return commandList_->Close();
    }
}