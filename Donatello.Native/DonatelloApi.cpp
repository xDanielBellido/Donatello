#include "pch.h"
#include "DonatelloApi.h"

#include "GraphicsDevice.h"
#include "CommandQueue.h"
#include "SwapChain.h"
#include "CommandContext.h"

#include <memory>
#include <new>
#include <cwchar>

struct DonatelloGraphics
{
    Donatello::GraphicsDevice device;
    Donatello::CommandQueue commandQueue;
    Donatello::SwapChain swapChain;
    Donatello::CommandContext commandContext;
};

int __cdecl Donatello_GetApiVersion()
{
    return 1;
}

std::int32_t __cdecl Donatello_CreateGraphics(DonatelloGraphics** outGraphics) noexcept
{
    if (outGraphics == nullptr)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    *outGraphics = nullptr;

    try
    {
        auto graphics = std::make_unique<DonatelloGraphics>();

        HRESULT result = graphics->device.Initialize();

        if (FAILED(result))
        {
            return static_cast<int32_t>(result);
        }

        result = graphics->commandQueue.Initialize(graphics->device.GetDevice());

        if (FAILED(result))
        {
            return static_cast<int32_t>(result);
        }

        result = graphics->commandContext.Initialize(graphics->device.GetDevice());

        if (FAILED(result))
        {
            return static_cast<std::int32_t>(result);
        }

        result = graphics->commandQueue.WaitForIdle();

        if (FAILED(result))
        {
            return static_cast<std::int32_t>(result);
        }

#if defined(_DEBUG)
        OutputDebugStringW(L"Donatello: initial GPU fence wait completed.\n");
#endif

        *outGraphics = graphics.release();

        return static_cast<int32_t>(S_OK);
    }
    catch (const std::bad_alloc&)
    {
        return static_cast<std::int32_t>(E_OUTOFMEMORY);
    }
    catch (...)
    {
        return static_cast<std::int32_t>(E_FAIL);
    }
}

void __cdecl Donatello_DestroyGraphics(DonatelloGraphics* graphics) noexcept
{
    delete graphics;
}

std::int32_t __cdecl Donatello_GetGraphicsAdapterName(DonatelloGraphics* graphics, wchar_t* buffer, std::uint32_t capacity) noexcept
{
    if (buffer == nullptr)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    if (capacity == 0)
    {
        return static_cast<std::int32_t>(E_INVALIDARG);
    }

    buffer[0] = L'\0';

    if (graphics == nullptr)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    DXGI_ADAPTER_DESC1 description{};
    const HRESULT result = graphics->device.GetAdapterDescription(description);

    if (FAILED(result))
    {
        return static_cast<std::int32_t>(result);
    }

    const std::size_t requiredCapacity = std::wcslen(description.Description) + 1;

    if (requiredCapacity > capacity)
    {
        return static_cast<std::int32_t>(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    }

    std::wmemcpy(buffer, description.Description, requiredCapacity);

    return static_cast<std::int32_t>(S_OK);
}

std::int32_t __cdecl Donatello_CreateSwapChain(DonatelloGraphics* graphics, void* window) noexcept
{
    if (graphics == nullptr)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    const HRESULT result = graphics->swapChain.Initialize(graphics->device.GetDevice(), graphics->device.GetFactory(), graphics->commandQueue.GetQueue(), static_cast<HWND>(window));

    return static_cast<std::int32_t>(result);
}

std::int32_t __cdecl Donatello_RenderFrame(DonatelloGraphics* graphics) noexcept
{
    if (!graphics)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    ID3D12Resource* backBuffer = graphics->swapChain.GetCurrentBackBuffer();

    if (!backBuffer)
    {
        return static_cast<std::int32_t>(E_UNEXPECTED);
    }

    // Finish previous work before reusing command memory.
    HRESULT result = graphics->commandQueue.WaitForIdle();

    if (FAILED(result))
    {
        return static_cast<std::int32_t>(result);
    }

    const float colour[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = graphics->swapChain.GetCurrentRenderTargetView();

    result = graphics->commandContext.RecordClear(backBuffer, rtv, colour);

    if (FAILED(result))
    {
        return static_cast<std::int32_t>(result);
    }

    result = graphics->commandQueue.Execute(graphics->commandContext.GetCommandList());

    if (FAILED(result))
    {
        return static_cast<std::int32_t>(result);
    }

    // Keep this first rendering path synchronous.
    result = graphics->commandQueue.WaitForIdle();

    if (FAILED(result))
    {
        return static_cast<std::int32_t>(result);
    }

    return static_cast<std::int32_t>(graphics->swapChain.Present());
}
