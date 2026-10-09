/*
    ===========================================================================
    CommandQueue
    ===========================================================================

    Owns the DirectX 12 graphics queue and a fence used to track its progress.

    WaitForIdle blocks the calling CPU thread until the GPU reaches a marker
    placed after previously submitted work on this queue.

    Releases its COM objects and closes its Windows event on destruction.
    GPU work must finish before dependent resources are destroyed.
*/

#pragma once

#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

namespace Donatello
{
    class CommandQueue final
    {
    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;
        Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
        HANDLE fenceEvent_ = nullptr;
        UINT64 fenceValue_ = 0;

    public:
        CommandQueue() = default;
        ~CommandQueue();

        CommandQueue(const CommandQueue&) = delete;
        CommandQueue& operator=(const CommandQueue&) = delete;

        HRESULT             Initialize(ID3D12Device* device);
        HRESULT             WaitForIdle();
        HRESULT             Execute(ID3D12CommandList* commandList);

        ID3D12CommandQueue* GetQueue() const { return queue_.Get(); }
    };
}