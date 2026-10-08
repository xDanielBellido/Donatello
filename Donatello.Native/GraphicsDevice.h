#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Donatello
{
    class GraphicsDevice final
    {
    public:
        GraphicsDevice() = default;
        ~GraphicsDevice() = default;

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator=(const GraphicsDevice&) = delete;

        HRESULT Initialize();
        HRESULT GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const;

    private:
        Microsoft::WRL::ComPtr<IDXGIFactory6> factory_;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter_;
        Microsoft::WRL::ComPtr<ID3D12Device> device_;
    };
}
