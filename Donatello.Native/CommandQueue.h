/*
    ===========================================================================
    CommandQueue
    ===========================================================================

    Owns the DirectX 12 graphics queue that will receive work for the GPU.

    Creates the queue using an existing graphics device. ComPtr releases the
    queue automatically when this object is destroyed.

    Does not yet submit commands or wait for GPU completion.
*/

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace Donatello
{
    class CommandQueue final
    {
    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;

    public:
        CommandQueue() = default;
        ~CommandQueue() = default;

        CommandQueue(const CommandQueue&) = delete;
        CommandQueue& operator=(const CommandQueue&) = delete;

        HRESULT                 Initialize(ID3D12Device* device);

        ID3D12CommandQueue*     GetQueue() const { return queue_.Get(); }
    };
}