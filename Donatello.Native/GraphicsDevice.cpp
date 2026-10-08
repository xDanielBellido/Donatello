#include "pch.h"
#include "GraphicsDevice.h"

namespace Donatello
{
    HRESULT GraphicsDevice::Initialize()
    {
        // Avoid initialising an already-created device.
        if (device_)
        {
            return S_OK;
        }

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