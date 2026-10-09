#include "pch.h"
#include "CommandQueue.h"

namespace Donatello
{
    HRESULT CommandQueue::Initialize(ID3D12Device* device)
    {
        if (!device)
        {
            return E_POINTER;
        }

        if (queue_)
        {
            return S_OK;
        }

        D3D12_COMMAND_QUEUE_DESC description{};
        description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        description.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        description.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        description.NodeMask = 0;

        Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue;
        const HRESULT result = device->CreateCommandQueue(&description, IID_PPV_ARGS(queue.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        queue_ = queue;

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: graphics command queue created.\n");
#endif

        return S_OK;
    }
}