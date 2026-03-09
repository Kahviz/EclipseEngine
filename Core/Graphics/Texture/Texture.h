#pragma once
#include <string>
#include <filesystem>
#include <wrl/client.h>
#include <d3d11.h>
#include "ErrorHandling/ErrorMessage.h"
#include "GLOBALS.h"
#include "Releaser.h"
#include "Debugging/Functions/UGE_ASSERTS.h"

class Dx11Renderer;

using namespace Microsoft::WRL;

class Texture {
public:

#if DIRECTX11 == 1
    Texture() : pTexture(nullptr) {}

    ID3D11ShaderResourceView* Load(std::string path, Dx11Renderer& dx11Renderer);

    ID3D11ShaderResourceView* GetSRV() const {
        return pTexture.Get();
    }

    ID3D11ShaderResourceView* const* GetAddressOf() const {
        return pTexture.GetAddressOf();
    }

    const ComPtr<ID3D11ShaderResourceView>& GetTextureComPtr() const {
        return pTexture;
    }

    ComPtr<ID3D11ShaderResourceView>& GetTextureComPtr() {
        return pTexture;
    }
#endif
    bool IsLoaded() {
        return Loaded;
    }
private:
    bool Loaded = false;
#if DIRECTX11 == 1
    ComPtr<ID3D11ShaderResourceView> pTexture;
#endif
};