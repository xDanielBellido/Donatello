#include "pch.h"
#include "DonatelloApi.h"

#include "Renderer.h"
#include "DemoContent.h"

#include <memory>
#include <new>
#include <cwchar>

struct DonatelloGraphics
{
    Donatello::Renderer renderer;
    Donatello::DemoContent demo;

    ~DonatelloGraphics() noexcept
    {
        const HRESULT result = renderer.WaitForIdle();

        if (FAILED(result))
        {
            wchar_t message[128]{};
            swprintf_s(message, L"Donatello: GPU shutdown wait failed with HRESULT 0x%08X.\n", static_cast<unsigned int>(result));
            OutputDebugStringW(message);
        }
    }
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
        HRESULT result = graphics->renderer.Initialize();

        if (FAILED(result))
        {
            return static_cast<std::int32_t>(result);
        }

        result = graphics->demo.Initialize(graphics->renderer.GetDevice());

        if (FAILED(result))
        {
            return static_cast<std::int32_t>(result);
        }

        *outGraphics = graphics.release();
        return static_cast<std::int32_t>(S_OK);
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
    const HRESULT result = graphics->renderer.GetAdapterDescription(description);

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

    return static_cast<std::int32_t>(graphics->renderer.CreateViewport(static_cast<HWND>(window)));
}

std::int32_t __cdecl Donatello_RenderFrame(DonatelloGraphics* graphics) noexcept
{
    if (graphics == nullptr)
    {
        return static_cast<std::int32_t>(E_POINTER);
    }

    const Donatello::RenderItem items[] = { graphics->demo.GetRenderItem() };

    return static_cast<std::int32_t>(graphics->renderer.RenderFrame(items));
}