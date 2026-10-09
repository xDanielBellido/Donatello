#include "pch.h"
#include "Renderer.h"

#include "ShaderCompiler.h"

#include <filesystem>
#include <cstring>

namespace Donatello
{
    
    HRESULT Renderer::Initialize()
    {
        HRESULT result = device_.Initialize();

        if (FAILED(result))
        {
            return result;
        }

        result = CompileShaders();

        if (FAILED(result))
        {
            return result;
        }

        result = CreatePipelineState();

        if (FAILED(result))
        {
            return result;
        }

        result = CreateVertexBuffer();

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.Initialize(device_.GetDevice());

        if (FAILED(result))
        {
            return result;
        }

        result = commandContext_.Initialize(device_.GetDevice());

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: initial GPU fence wait completed.\n");
#endif

        return S_OK;
    }

    HRESULT Renderer::CreateViewport(HWND window)
    {
        return swapChain_.Initialize(device_.GetDevice(), device_.GetFactory(), commandQueue_.GetQueue(), window);
    }

    HRESULT Renderer::GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const
    {
        return device_.GetAdapterDescription(description);
    }

    HRESULT Renderer::RenderFrame()
    {
        // Finish previous work before resizing or reusing command memory.
        HRESULT result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

        result = swapChain_.ResizeToWindow(device_.GetDevice());

        if (result == S_FALSE)
        {
            return S_FALSE;
        }

        if (FAILED(result))
        {
            return result;
        }

        ID3D12Resource* backBuffer = swapChain_.GetCurrentBackBuffer();

        if (!backBuffer)
        {
            return E_UNEXPECTED;
        }

        const float colour[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv = swapChain_.GetCurrentRenderTargetView();

        result = commandContext_.BeginFrame(backBuffer, rtv, colour);

        if (FAILED(result))
        {
            return result;
        }

        result = commandContext_.Draw(graphicsPipeline_, mesh_, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        if (FAILED(result))
        {
            commandContext_.EndFrame(backBuffer);
            return result;
        }

        result = commandContext_.EndFrame(backBuffer);

        if (FAILED(result))
        {
            return result;
        }

        result = commandQueue_.Execute(commandContext_.GetCommandList());

        if (FAILED(result))
        {
            return result;
        }

        // Keep this first rendering path synchronous.
        result = commandQueue_.WaitForIdle();

        if (FAILED(result))
        {
            return result;
        }

        return swapChain_.Present();
    }

    HRESULT Renderer::CompileShaders()
    {
        wchar_t executablePath[MAX_PATH]{};
        DWORD length = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);

        if (length == 0)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        if (length >= MAX_PATH)
        {
            return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        }

        const auto shaderPath = std::filesystem::path(executablePath).parent_path() / L"Triangle.hlsl";

        UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined(_DEBUG)
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        auto vertexShader = ShaderCompiler::Compile(shaderPath.c_str(), "VSMain", "vs_5_0", flags);

        if (vertexShader.diagnostics)
        {
            OutputDebugStringA(static_cast<const char*>(vertexShader.diagnostics->GetBufferPointer()));
        }

        if (FAILED(vertexShader.result))
        {
            return vertexShader.result;
        }

        auto pixelShader = ShaderCompiler::Compile(shaderPath.c_str(), "PSMain", "ps_5_0", flags);

        if (pixelShader.diagnostics)
        {
            OutputDebugStringA(static_cast<const char*>(pixelShader.diagnostics->GetBufferPointer()));
        }

        if (FAILED(pixelShader.result))
        {
            return pixelShader.result;
        }

        vertexShader_ = vertexShader.bytecode;
        pixelShader_ = pixelShader.bytecode;

        OutputDebugStringW(L"Donatello: triangle shaders compiled successfully.\n");

        return S_OK;
    }

    HRESULT Renderer::CreatePipelineState()
    {
        const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC description{};

        description.VS = { vertexShader_->GetBufferPointer(), vertexShader_->GetBufferSize() };
        description.PS = { pixelShader_->GetBufferPointer(), pixelShader_->GetBufferSize() };
        description.InputLayout = { inputLayout, 2 };

        // Draw filled triangles without discarding either face.
        description.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        description.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        description.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        description.RasterizerState.DepthClipEnable = TRUE;

        // Write the shader's colour directly into the render target.
        auto& blend = description.BlendState.RenderTarget[0];
        blend.BlendEnable = FALSE;
        blend.LogicOpEnable = FALSE;
        blend.SrcBlend = D3D12_BLEND_ONE;
        blend.DestBlend = D3D12_BLEND_ZERO;
        blend.BlendOp = D3D12_BLEND_OP_ADD;
        blend.SrcBlendAlpha = D3D12_BLEND_ONE;
        blend.DestBlendAlpha = D3D12_BLEND_ZERO;
        blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend.LogicOp = D3D12_LOGIC_OP_NOOP;
        blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        // This first triangle does not use a depth or stencil buffer.
        auto& depth = description.DepthStencilState;
        depth.DepthEnable = FALSE;
        depth.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depth.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        depth.StencilEnable = FALSE;
        depth.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
        depth.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
        depth.FrontFace = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
        depth.BackFace = depth.FrontFace;

        // Match the swap chain's format and sample count.
        description.NumRenderTargets = 1;
        description.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        description.SampleDesc.Count = 1;
        description.SampleMask = 0xffffffffu;

        D3D12_ROOT_SIGNATURE_DESC rootDescription{};
        rootDescription.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        HRESULT result = graphicsPipeline_.Initialize(device_.GetDevice(), rootDescription, description);

        if (FAILED(result))
        {
            return result;
        }

        OutputDebugStringW(L"Donatello: graphics pipeline created successfully.\n");

        return S_OK;
    }
    
    HRESULT Renderer::CreateVertexBuffer()
    {
        struct Vertex
        {
            float position[3];
            float colour[3];
        };

        const Vertex vertices[] =
        {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
        };

        const UINT vertexCount = static_cast<UINT>(sizeof(vertices) / sizeof(vertices[0]));
        const UINT vertexStride = static_cast<UINT>(sizeof(Vertex));

        HRESULT result = mesh_.Initialize(device_.GetDevice(), vertices, vertexCount, vertexStride);

        if (FAILED(result))
        {
            return result;
        }

        OutputDebugStringW(L"Donatello: GPU mesh created successfully.\n");

        return S_OK;
    }
}