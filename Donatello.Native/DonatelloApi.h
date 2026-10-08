/*
    ===========================================================================
    DonatelloApi
    ===========================================================================

    Declares the C++ functions that the C# application can call through P/Invoke.

    Provides operations to query the API version, create and destroy a graphics
    context, and retrieve the selected adapter's name.

    Keeps the context's contents hidden from C#. The application receives an
    opaque handle and passes it back when calling these functions.

    These declarations must match the signatures in NativeMethods.cs.
*/

#pragma once

#include <cstdint>

struct DonatelloGraphics;

extern "C" __declspec(dllexport) int __cdecl 
Donatello_GetApiVersion();

extern "C" __declspec(dllexport) std::int32_t __cdecl 
Donatello_CreateGraphics(DonatelloGraphics** outGraphics) noexcept;

extern "C" __declspec(dllexport) void __cdecl 
Donatello_DestroyGraphics(DonatelloGraphics* graphics) noexcept;

extern "C" __declspec(dllexport) std::int32_t __cdecl 
Donatello_GetGraphicsAdapterName(DonatelloGraphics* graphics, wchar_t* buffer, std::uint32_t capacity) noexcept;