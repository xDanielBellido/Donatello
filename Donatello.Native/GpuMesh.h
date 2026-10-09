/*
    ===========================================================================
    GpuMesh
    ===========================================================================

    Owns a vertex buffer and its view for non-indexed drawing.
    Copies the supplied vertex data into an upload resource during initialisation.
*/

#pragma once

#include <d3d12.h>
#include <wrl/client.h>

namespace Donatello
{
    class GpuMesh final
    {
    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
        UINT vertexCount_ = 0;

    public:
        GpuMesh() = default;
        ~GpuMesh() = default;

        GpuMesh(const GpuMesh&) = delete;
        GpuMesh& operator=(const GpuMesh&) = delete;

        HRESULT Initialize(ID3D12Device* device, const void* vertices, UINT vertexCount, UINT vertexStride);

        const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }
        UINT GetVertexCount() const { return vertexCount_; }
    };
}