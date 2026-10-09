/*
    ===========================================================================
    RenderItem
    ===========================================================================

    Describes a non-indexed draw using an existing pipeline and GPU mesh.
    Referenced resources must remain alive until GPU execution finishes.
*/

#pragma once

#include <d3d12.h>

namespace Donatello
{
    class GraphicsPipeline;
    class GpuMesh;

    struct RenderItem
    {
        const GraphicsPipeline& pipeline;
        const GpuMesh& mesh;
        D3D_PRIMITIVE_TOPOLOGY topology;
    };
}