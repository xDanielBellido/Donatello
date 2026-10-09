/*
    ===========================================================================
    ShaderCompiler
    ===========================================================================

    Compiles an HLSL entry point into bytecode and returns compiler diagnostics.
    The caller chooses the file, shader profile and compilation options.
*/

#pragma once

#include <d3dcompiler.h>
#include <wrl/client.h>

namespace Donatello
{
    struct ShaderCompilation
    {
        HRESULT result = E_FAIL;
        Microsoft::WRL::ComPtr<ID3DBlob> bytecode;
        Microsoft::WRL::ComPtr<ID3DBlob> diagnostics;
    };

    namespace ShaderCompiler
    {
        ShaderCompilation Compile(const wchar_t* filePath, const char* entryPoint, const char* target, UINT flags);
    }
}