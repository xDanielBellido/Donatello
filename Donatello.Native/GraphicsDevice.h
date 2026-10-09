/*
    ===========================================================================
    GraphicsDevice
    ===========================================================================

    Selects a graphics adapter and creates the DirectX 12 device used to create
    GPU resources.

    Owns the DXGI factory, selected adapter and device through ComPtr, which
    releases their COM references automatically when this object is destroyed.

    Provides access to the selected adapter's information.
    Does not record commands, submit GPU work or present images.
*/

#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace Donatello
{
    class GraphicsDevice final
    {
    private:
        Microsoft::WRL::ComPtr<IDXGIFactory6> factory_;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter_;
        Microsoft::WRL::ComPtr<ID3D12Device> device_;



    public:
                            // Construction and ownership
                            GraphicsDevice() = default;
                            ~GraphicsDevice() = default;

                            GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice&     operator=(const GraphicsDevice&) = delete;


        HRESULT             Initialize();
        HRESULT             GetAdapterDescription(DXGI_ADAPTER_DESC1& description) const;


        ID3D12Device*       GetDevice() const { return device_.Get(); }

    };
}
