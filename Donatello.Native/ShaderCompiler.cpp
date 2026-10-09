#include "pch.h"
#include "ShaderCompiler.h"

namespace Donatello::ShaderCompiler
{
    ShaderCompilation Compile(const wchar_t* filePath, const char* entryPoint, const char* target, UINT flags)
    {
        ShaderCompilation compilation;

        if (!filePath || !entryPoint || !target)
        {
            compilation.result = E_POINTER;
            return compilation;
        }

        compilation.result = D3DCompileFromFile(filePath, nullptr, nullptr, entryPoint, target, flags, 0, compilation.bytecode.GetAddressOf(), compilation.diagnostics.GetAddressOf());

        return compilation;
    }
}