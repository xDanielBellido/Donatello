#include "pch.h"
#include "CommandQueue.h"

#include <dxgi.h>

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
        HRESULT result = device->CreateCommandQueue(&description, IID_PPV_ARGS(queue.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        Microsoft::WRL::ComPtr<ID3D12Fence> fence;
        result = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        HANDLE fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

        if (!fenceEvent)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        queue_ = queue;
        fence_ = fence;
        fenceEvent_ = fenceEvent;
        fenceValue_ = 0;

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: graphics command queue and fence created.\n");
#endif

        return S_OK;
    }

    CommandQueue::~CommandQueue()
    {
        if (fenceEvent_)
        {
            CloseHandle(fenceEvent_);
        }
    }

    HRESULT CommandQueue::WaitForIdle()
    {
        if (!queue_ || !fence_ || !fenceEvent_)
        {
            return E_UNEXPECTED;
        }

        const UINT64 targetValue = fenceValue_ + 1;
        HRESULT result = queue_->Signal(fence_.Get(), targetValue);

        if (FAILED(result))
        {
            return result;
        }

        fenceValue_ = targetValue;

        UINT64 completedValue = fence_->GetCompletedValue();

        if (completedValue == UINT64_MAX)
        {
            return DXGI_ERROR_DEVICE_REMOVED;
        }

        if (completedValue >= targetValue)
        {
            return S_OK;
        }

        result = fence_->SetEventOnCompletion(targetValue, fenceEvent_);

        if (FAILED(result))
        {
            return result;
        }

        const DWORD waitResult = WaitForSingleObject(fenceEvent_, INFINITE);

        if (waitResult == WAIT_FAILED)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (waitResult != WAIT_OBJECT_0)
        {
            return E_FAIL;
        }

        completedValue = fence_->GetCompletedValue();

        if (completedValue == UINT64_MAX)
        {
            return DXGI_ERROR_DEVICE_REMOVED;
        }

        return completedValue >= targetValue ? S_OK : E_FAIL;
    }

    HRESULT CommandQueue::Execute(ID3D12CommandList* commandList)
    {
        if (!commandList)
        {
            return E_POINTER;
        }

        if (!queue_)
        {
            return E_UNEXPECTED;
        }

        ID3D12CommandList* lists[] = { commandList };
        queue_->ExecuteCommandLists(1, lists);

        return S_OK;
    }

}