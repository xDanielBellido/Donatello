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