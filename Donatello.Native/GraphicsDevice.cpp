#include "pch.h"
#include "GraphicsDevice.h"

#include <d3d12sdklayers.h>

namespace Donatello
{
    HRESULT GraphicsDevice::Initialize()
    {
        // Avoid initialising an already-created device.
        if (device_)
        {
            return S_OK;
        }

#if defined(_DEBUG)
        Microsoft::WRL::ComPtr<ID3D12Debug> debugController;

        const HRESULT debugResult = D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf()));

        if (FAILED(debugResult))
        {
            OutputDebugStringW(L"Donatello: DirectX 12 debug layer is unavailable.\n");
            return debugResult;
        }

        debugController->EnableDebugLayer();

        OutputDebugStringW(L"Donatello: DirectX 12 debug layer enabled.\n");
#endif


        Microsoft::WRL::ComPtr<IDXGIFactory6> factory;

        HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(factory.GetAddressOf()));

        if (FAILED(result))
        {
            return result;
        }

        HRESULT lastDeviceError = DXGI_ERROR_UNSUPPORTED;

        for (UINT index = 0; ; ++index)
        {
            Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;

            result = factory->EnumAdapterByGpuPreference(index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.GetAddressOf()));

            if (result == DXGI_ERROR_NOT_FOUND)
            {
                break;
            }

            if (FAILED(result))
            {
                return result;
            }

            DXGI_ADAPTER_DESC1 description{};

            result = adapter->GetDesc1(&description);

            if (FAILED(result))
            {
                return result;
            }

            // This first implementation only considers hardware adapters.
            if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                continue;
            }

            Microsoft::WRL::ComPtr<ID3D12Device> device;

            result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf()));

            if (FAILED(result))
            {
                lastDeviceError = result;
                continue;
            }

#if defined(_DEBUG)
            Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue;

            result = device.As(&infoQueue);

            if (FAILED(result))
            {
                return result;
            }

            result = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);

            if (FAILED(result))
            {
                return result;
            }

            result = infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

            if (FAILED(result))
            {
                return result;
            }

            OutputDebugStringW(L"Donatello: DirectX 12 diagnostic breaks configured.\n");
#endif

            // Keep the objects only after initialisation succeeds.
            factory_ = factory;
            adapter_ = adapter;
            device_ = device;

            return S_OK;
        }

        return lastDeviceError;
    }

    HRESULT GraphicsDevice::GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const
    {
        description = {};

        if (!adapter_)
        {
            return E_UNEXPECTED;
        }

        return adapter_->GetDesc1(&description);
    }
}