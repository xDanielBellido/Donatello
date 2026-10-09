/*
    ===========================================================================
    CommandContext
    ===========================================================================

    Owns a DirectX 12 graphics command list and its command allocator.

    The list records GPU commands. The allocator manages the memory used
    to store them. ComPtr releases both objects automatically.

    Initialises an empty, closed list. Does not submit work to the GPU.
    Records commands to clear a back buffer and prepare it for presentation.
    The caller must wait for previous GPU work before recording again.

    Does not submit work to the GPU.
*/

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace Donatello
{
    class CommandContext final
    {
    private:
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    public:
        CommandContext() = default;
        ~CommandContext() = default;

        CommandContext(const CommandContext&) = delete;
        CommandContext& operator=(const CommandContext&) = delete;

        HRESULT             Initialize(ID3D12Device* device);

        HRESULT                     RecordClear(ID3D12Resource* backBuffer, D3D12_CPU_DESCRIPTOR_HANDLE rtv, const float* colour);
        ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
    };
}