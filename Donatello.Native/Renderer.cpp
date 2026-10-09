#include "pch.h"
#include "Renderer.h"

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

        result = CreateRootSignature();

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

        result = commandContext_.RecordTriangle(backBuffer, rtv, colour, pipelineState_.Get(), rootSignature_.Get(), vertexBufferView_);

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

        Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
        Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
        Microsoft::WRL::ComPtr<ID3DBlob> messages;

        HRESULT result = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", flags, 0, vertexShader.GetAddressOf(), messages.GetAddressOf());

        if (messages)
        {
            OutputDebugStringA(static_cast<const char*>(messages->GetBufferPointer()));
        }

        if (FAILED(result))
        {
            return result;
        }

        messages.Reset();

        result = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", flags, 0, pixelShader.GetAddressOf(), messages.GetAddressOf());

        if (messages)
        {
            OutputDebugStringA(static_cast<const char*>(messages->GetBufferPointer()));
        }

        if (FAILED(result))
        {
            return result;
        }

        vertexShader_ = vertexShader;
        pixelShader_ = pixelShader;

        OutputDebugStringW(L"Donatello: triangle shaders compiled successfully.\n");

        return S_OK;
    }

    HRESULT Renderer::CreateRootSignature()
    {
        D3D12_ROOT_SIGNATURE_DESC description{};
        description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        Microsoft::WRL::ComPtr<ID3DBlob> serialised;
        Microsoft::WRL::ComPtr<ID3DBlob> messages;

        HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1, serialised.GetAddressOf(), messages.GetAddressOf());

        if (messages)
        {
            OutputDebugStringA(static_cast<const char*>(messages->GetBufferPointer()));
        }

        if (FAILED(result))
        {
            return result;
        }

        Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;

        result = device_.GetDevice()->CreateRootSignature(0, serialised->GetBufferPointer(), serialised->GetBufferSize(), IID_PPV_ARGS(rootSignature.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        rootSignature_ = rootSignature;

        OutputDebugStringW(L"Donatello: triangle root signature created successfully.\n");

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

        description.pRootSignature = rootSignature_.Get();
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

        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

        HRESULT result = device_.GetDevice()->CreateGraphicsPipelineState(&description, IID_PPV_ARGS(pipelineState.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        pipelineState_ = pipelineState;

        OutputDebugStringW(L"Donatello: triangle pipeline state created successfully.\n");

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

        D3D12_HEAP_PROPERTIES heap{};
        heap.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap.CreationNodeMask = 1;
        heap.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC description{};
        description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        description.Width = sizeof(vertices);
        description.Height = 1;
        description.DepthOrArraySize = 1;
        description.MipLevels = 1;
        description.SampleDesc.Count = 1;
        description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;

        HRESULT result = device_.GetDevice()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBuffer.GetAddressOf()));

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

        std::memcpy(mappedData, vertices, sizeof(vertices));

        const D3D12_RANGE writtenRange{ 0, sizeof(vertices) };
        vertexBuffer->Unmap(0, &writtenRange);

        D3D12_VERTEX_BUFFER_VIEW view{};
        view.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
        view.SizeInBytes = static_cast<UINT>(sizeof(vertices));
        view.StrideInBytes = static_cast<UINT>(sizeof(Vertex));

        vertexBuffer_ = vertexBuffer;
        vertexBufferView_ = view;

        OutputDebugStringW(L"Donatello: triangle vertex buffer created successfully.\n");

        return S_OK;
    }
}