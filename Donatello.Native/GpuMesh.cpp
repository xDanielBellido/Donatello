#include "pch.h"
#include "GpuMesh.h"

#include <climits>
#include <cstring>

namespace Donatello
{
    HRESULT GpuMesh::Initialize(ID3D12Device* device, const void* vertices, UINT vertexCount, UINT vertexStride)
    {
        if (!device || !vertices)
        {
            return E_POINTER;
        }

        if (vertexCount == 0 || vertexStride == 0)
        {
            return E_INVALIDARG;
        }

        if (vertexBuffer_)
        {
            return E_UNEXPECTED;
        }

        const UINT64 totalBytes = static_cast<UINT64>(vertexCount) * vertexStride;

        if (totalBytes > UINT_MAX)
        {
            return E_INVALIDARG;
        }

        const UINT bufferSize = static_cast<UINT>(totalBytes);

        D3D12_HEAP_PROPERTIES heap{};
        heap.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap.CreationNodeMask = 1;
        heap.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC description{};
        description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        description.Width = bufferSize;
        description.Height = 1;
        description.DepthOrArraySize = 1;
        description.MipLevels = 1;
        description.SampleDesc.Count = 1;
        description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;

        HRESULT result = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBuffer.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        void* mappedData = nullptr;
        const D3D12_RANGE readRange{ 0, 0 };

        result = vertexBuffer->Map(0, &readRange, &mappedData);

        if (FAILED(result))
        {
            return result;
        }

        std::memcpy(mappedData, vertices, bufferSize);

        const D3D12_RANGE writtenRange{ 0, bufferSize };
        vertexBuffer->Unmap(0, &writtenRange);

        D3D12_VERTEX_BUFFER_VIEW view{};
        view.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        view.SizeInBytes = bufferSize;
        view.StrideInBytes = vertexStride;

        vertexBuffer_ = vertexBuffer;
        vertexBufferView_ = view;
        vertexCount_ = vertexCount;

        return S_OK;
    }
}